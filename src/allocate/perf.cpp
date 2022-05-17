#include "perf.h"

#include <cmath>
#include <limits>
#include <map>
#include <queue>
using std::cerr;
using std::endl;
using std::map;
using std::priority_queue;

namespace hls {

typedef pair<int, vector<int>> AdjacentNode;
typedef map<int, AdjacentNode> AdjacentList;

static AdjacentList build_induced_graph(int bbid, const HLSInput &hin) {
    AdjacentList list;
    const BasicBlock &bb = hin.blocks[bbid];

    // add nodes
    for (auto v : bb.ops)
        list.insert(std::make_pair(v, AdjacentNode(0, vector<int>())));

    // add edges
    for (auto v : bb.ops) {
        const auto &op = hin.operations[v];
        if (hin.get_opcate(v) == OP_PHI)  // ignore phi nodes inputs
            continue;
        for (auto u : op.inputs) {
            if (list.count(u)) {  // prev vertex, ignore -1 automatically
                list[v].first++;  // in degree ++
                list[u].second.push_back(v);  // append to outs
            }
        }
    }

    return list;
}

// setup depedencies for all types of operations
void AbstractedCDFG::setup() {
    AdjacentList induced_graph = build_induced_graph(bbid, *hin);

    // bfs the graph
    queue<int> bfs;
    map<int, int> depths;
    for (auto v : hin->blocks[bbid].ops) depths.insert(std::make_pair(v, -1));
    // insert nodes with no preds
    for (auto it = induced_graph.begin(); it != induced_graph.end(); it++) {
        if (it->second.first == 0) {
            depths[it->first] = 0;
            bfs.push(it->first);
        }
    }
    while (!bfs.empty()) {
        auto v = bfs.front();
        bfs.pop();
        // finish this operation and ready its successors
        for (auto u : induced_graph[v].second) {
            // update depth of the same op
            if (hin->operations[u].optype == hin->operations[v].optype) {
                depths[u] = std::max(depths[u], depths[v] + 1);
            }
            // schedule op with all preds finished
            if ((--induced_graph[u].first) == 0) {
                depths[u] = std::max(depths[u], 0);
                bfs.push(u);
            }
        }
    }

    // write bfs result to dependencies
    for (auto it = depths.begin(); it != depths.end(); it++) {
        int optype = hin->operations[it->first].optype;
        int depth = it->second;
        if (depth == -1) {
            std::cerr << "Failure while building CDFG!" << std::endl;
            return;
        }
        auto &cur_dependency = dependencies[optype];
        for (int i = cur_dependency.size(); i < depth + 1; i++)
            cur_dependency.push_back(0);
        cur_dependency[depth]++;
    }
}

void AbstractedCDFG::print() {
    using std::cout;
    using std::endl;

    cout << "Abstracted CDFG of Basic Block: " << bbid << endl;
    for (auto d : dependencies) {
        for (auto n : d) {
            cout << n << ' ';
        }
        cout << endl;
    }
    cout << endl;
}

// Estimate expected performance of optype in this basic block,
// given its allocated resource type and number of resources.
// A rough estimation: optype must finish ops of depths k before
// executing ops of depths k+1
// Return an expected latency on success, negative values on error.
float AbstractedCDFG::estimate_perf(int optype, const ResourceType &rtype,
                                    int num) {
    float target_cp = hin->target_cp;
    float lat = rtype.latency;
    float delay = rtype.delay;
    float res = 0;
    for (int i = 0; i < dependencies[optype].size(); i++) {
        int n = dependencies[optype][i];
        float tmp = 0;
        if (rtype.is_pipelined) {
            tmp = target_cp * (lat + floorf((float)n / num)) + delay;
        } else {
            tmp = ceilf((float)n / num) * (target_cp * lat + delay);
        }
        res += tmp;
    }
    res *= hin->blocks[bbid].exp_times;
    return res;
}

// Run estimation on all basic blocks
// Return an expected latency
float PerfAllocator::estimate_perf(int optype, const ResourceType &rtype,
                                   int num) {
    float exp_perf = 0;
    for (int i = 0; i < n_block; i++) {
        exp_perf += cdfgs[i].estimate_perf(optype, rtype, num);
    }
    return exp_perf;
}

// Change types to better ones
void PerfAllocator::allocate_type(int area_limit) {
#ifdef DEBUG_HLS_ALLOCATE_PERF
    print(true);
#endif

    // expected performance of each type
    // for uncompatible operations, the performance is infinity
    int n_resource_type = hin->n_resource_type;
    vector<vector<float>> exp_perfs(
        n_op_type,
        vector<float>(n_resource_type, std::numeric_limits<float>::infinity()));

    for (int optype = 0; optype < n_op_type; optype++) {
        for (int rtid = 0; rtid < n_resource_type; rtid++) {
            const ResourceType &rtype = hin->resource_types[rtid];

            // check compatibility
            bool is_comp = false;
            for (auto comp_op : rtype.comp_ops) {
                if (comp_op == optype) {
                    is_comp = true;
                    break;
                }
            }
            if (!is_comp) continue;

            // get expected performance on each basic block
            exp_perfs[optype][rtid] = estimate_perf(optype, rtype, 1);
        }
    }

    // DP for the best allocation
    vector<vector<float>> dp(
        n_op_type,
        vector<float>(area_limit + 1, std::numeric_limits<float>::infinity()));
    vector<vector<int>> footprint(n_op_type, vector<int>(area_limit + 1, -1));
    // dp
    for (int optype = 0; optype < n_op_type; optype++) {
        // for operation without allocation, skip dp
        auto op_cate = hin->op_types[optype];
        if (op_cate == OP_ALLOCA || op_cate == OP_BRANCH || op_cate == OP_PHI) {
            for (int area = 0; area <= area_limit; area++) {
                dp[optype][area] = optype == 0 ? 0 : dp[optype - 1][area];
                footprint[optype][area] = -1;
            }
        } else {
            for (int rtid = 0; rtid < n_resource_type; rtid++) {
                const ResourceType &rtype = hin->resource_types[rtid];
                for (int area = rtype.area; area <= area_limit; area++) {
                    float old_perf = dp[optype][area];
                    float new_perf =
                        (optype == 0 ? 0 : dp[optype - 1][area - rtype.area]) +
                        exp_perfs[optype][rtid];
                    if (new_perf < old_perf) {
                        dp[optype][area] = new_perf;
                        footprint[optype][area] = rtid;
                    }
                }
            }
        }
    }

    // record the best result
    int area = area_limit;
    for (int i = n_op_type - 1; i >= 0; i--) {
        int rtid = footprint[i][area];
        ot2rtid[i] = rtid;
        if (rtid != -1) {
            const ResourceType &rtype = hin->resource_types[rtid];
            area -= rtype.area;
        } else {
            auto op_cate = hin->op_types[i];
            if (hin->need_schedule(op_cate))
                cerr << "Optype " << i << " not allocated with rtype!" << endl;
        }
    }

    // print(true);
}

void PerfAllocator::allocate_inst() {
    priority_queue<IncrNode, vector<IncrNode>> q;

    // initialize q and insts
    int total_area = 0;
    for (int otid = 0; otid < n_op_type; otid++) {
        auto opcate = hin->op_types[otid];
        if (hin->need_bind(opcate)) {
            const auto &rtype = hin->resource_types[ot2rtid[otid]];
            total_area += rtype.area;

            IncrNode node;
            node.optype = otid;
            node.area = rtype.area;
            node.num = ++insts[otid];
            node.oldperf = estimate_perf(otid, rtype, 1);
            node.newperf = estimate_perf(otid, rtype, 2);
            q.push(node);
        } else {
            insts[otid] = 0;
        }
    }

    // allocate more insts
    while (!q.empty()) {
        auto node = q.top();
        q.pop();

        if (total_area + node.area <= hin->area_limit) {
            // update on success
            total_area += node.area;
            node.num = ++insts[node.optype];

            // debug: print result
            // cerr << "Perf gain on " << node.num << "-th inst of  optype "
            //      << node.optype << " = "
            //      << (node.oldperf - node.newperf) / node.area << endl;

            // push back new option
            node.oldperf = node.newperf;
            const auto &rtype = hin->resource_types[ot2rtid[node.optype]];
            node.newperf =
                estimate_perf(node.optype, rtype, insts[node.optype]);
            q.push(node);
        }
    }

    // print(true);
}


// maximize performance gain, minimize area cost
bool IncrNode::operator<(const IncrNode &x) const {
    auto gain = (oldperf - newperf) / area;
    auto x_gain = (x.oldperf - x.newperf) / x.area;
    if (fabs(gain - x_gain) < 1) {
        return num > x.num;  // smaller num, higher prior
    } else {
        return gain < x_gain;  // higher gain, higher prior
    }
}

}  // namespace hls

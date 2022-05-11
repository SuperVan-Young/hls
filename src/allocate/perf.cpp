#include "perf.h"

#include <map>
using std::map;

// #define DEBUG

namespace hls {

AdjacentList build_induced_graph(int bbid, const HLSInput &hin) {
    AdjacentList list;
    const BasicBlock &bb = hin.blocks[bbid];

    // add nodes
    for (auto v : bb.ops)
        list.insert(std::make_pair(v, AdjacentNode(0, vector<int>())));

    // add edges
    for (auto v : bb.ops) {
        const auto &op = hin.operations[v];
        if (hin.get_op_cate(&op) == OP_PHI)  // ignore phi nodes inputs
            continue;
        for (auto u : op.inputs) {
            if (list.count(u)) {  // prev vertex, ignore -1 automatically
                list[v].first++;  // in degree ++
                list[u].second.push_back(v);  // append to outs
            }
        }
    }

#ifdef DEBUG
    // display the result
    using std::cout;
    using std::endl;
    cout << "Adjacent List of bbid " << bbid << endl;
    for (auto it = list.begin(); it != list.end(); it++) {
        cout << it->first << " ";
        cout << "(" << it->second.first << "): ";
        for (auto v : it->second.second) cout << v << ' ';
        cout << endl;
    }
#endif

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

#ifdef DEBUG
    print();
#endif
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

void PerfAllocator::setup() {}

// Change types to better ones
void PerfAllocator::allocate_type() {}

void PerfAllocator::allocate_inst() {}

}  // namespace hls

#include "sdc.h"

using std::cerr;
using std::endl;

// #define DEBUG_HLS_SCHEDULE_SDC

namespace hls {

static inline int get_latency(const vector<int> &ot2rtid, const HLSInput *hin,
                              int opid) {
    const auto &op = hin->operations[opid];
    auto rtid = ot2rtid[op.optype];
    const auto &rt = hin->resource_types[rtid];
    return rt.latency;
}

// Returns a mapping from opid to an index in bb.ops
map<int, int> build_op2idx(const BasicBlock &bb) {
    map<int, int> res;
    int i = 0;
    for (auto v : bb.ops) res.insert(std::make_pair(v, i++));
    return res;
}

// Topology sort induced graph g, but output according to each operation type
// Returns 0 on success, -1 on errors (loops)
int topology_sort(AdjacentList g, const HLSInput &hin,
                  vector<vector<int>> &out) {
    out.resize(hin.n_op_type, vector<int>());
    int cnt = 0;

    queue<int> q_ready;

    for (auto it = g.begin(); it != g.end(); it++) {
        if (it->second.first == 0) {  // in degree == 0
            q_ready.push(it->first);
        }
    }

    while (!q_ready.empty()) {
        auto v = q_ready.front();
        q_ready.pop();
        cnt++;

        // Record finished nodes, but write to different groups
        auto otid = hin.operations[v].optype;
        out[otid].push_back(v);

        auto node = g[v];
        for (auto u : node.second) {    // out vertex
            if ((--g[u].first) == 0) {  // in degree
                q_ready.push(u);
            }
        }
    }

    if (cnt != g.size()) return -1;

#ifdef DEBUG_HLS_SCHEDULE_SDC
    for (int i = 0; i < hin.n_op_type; i++) {
        cerr << i << ": ";
        for (auto v : out[i]) cerr << v << ' ';
        cerr << endl;
    }
    cerr << endl;
#endif

    return 0;
}

int SDCScheduler::schedule_block(int bbid, map<int, int> &res) {
    const auto &bb = hin->blocks[bbid];
    int *colno = new int[bb.n_op_in_block + 1];  // ordered by bb.ops
    REAL *row = new REAL[bb.n_op_in_block + 1];
    int ret = 0;  // if ret == -1, will skip to cleaning up

    // build ILP model
    auto lp = make_lp(0, bb.n_op_in_block + 1);  // last column = x_end
    if (lp == 0) ret = -1;

#ifndef DEBUG_HLS_SCHEDULE_SDC
    set_verbose(lp, IMPORTANT);
#endif

    if (!ret)
        for (int i = 1; i <= bb.n_op_in_block + 1; i++)
            set_int(lp, i, TRUE);  // integer variables

    // Add constraints
    set_add_rowmode(lp, TRUE);
    if (!ret) ret = add_constraints(bbid, lp);

    // Set objective
    if (!ret) {
        set_add_rowmode(lp, FALSE);
        set_minim(lp);
        colno[0] = bb.n_op_in_block + 1;
        row[0] = 1;
        if (!set_obj_fnex(lp, 1, row, colno)) ret = -1;
    }

    // Run the model
    if (!ret) {
        int ret_lp = solve(lp);
        if (!(ret_lp == OPTIMAL || ret_lp == SUBOPTIMAL)) {
            ret = -1;
        }
    }

    // Write the result to res
    if (!ret) {
        get_variables(lp, row);
        int max_cycle = 0;
        for (int i = 0; i < bb.n_op_in_block; i++) {
            int op = bb.ops[i];
            int cycle = (int)row[i];

            // if don't need to schedule, please make it to -1!
            auto opcate = hin->get_opcate(op);
            if (hin->need_schedule(opcate)) {
                res.insert(std::make_pair(op, cycle));
                int latency = get_latency(ot2rtid, hin, op);
                max_cycle = std::max(max_cycle, cycle + latency + 1);
            } else {
                res.insert(std::make_pair(op, -1));
            }
        }
        ret = max_cycle;
    }

    // Clean up and return
    if (lp != 0) delete_lp(lp);
    delete[] colno;
    delete[] row;
    return ret;
}

// Add constraints to LP model.
// Return 0 on success, -1 on errors
int SDCScheduler::add_constraints(int bbid, lprec *lp) {
    const auto &bb = hin->blocks[bbid];
    auto op2idx = build_op2idx(bb);

    int *colno = new int[bb.n_op_in_block];
    REAL *row = new REAL[bb.n_op_in_block];
    int ret = 0;

    AdjacentList g = build_induced_graph(bbid, *hin);

    // Dependence constraints & Optimization constraints
    for (auto it = g.begin(); it != g.end(); it++) {
        int opid = it->first;

        // for ops scheduled to -1, ignore them and their out edges
        auto opcate = hin->get_opcate(opid);
        if (!hin->need_schedule(opcate)) continue;

        // add dependency on the out edges
        for (auto out : it->second.second) {
            if (!ret) {
                // x_out - x_opid >= latency + 1
                colno[0] = op2idx[out] + 1;
                colno[1] = op2idx[opid] + 1;
                row[0] = 1;
                row[1] = -1;
                int latency = get_latency(ot2rtid, hin, opid);
                // Notice that chaining is not allowed now
                if (!add_constraintex(lp, 2, row, colno, GE, latency + 1))
                    ret = -1;
            }
        }

        // add optimization goal
        if (!ret) {
            // x_opid - x_end <= 0
            colno[0] = op2idx[opid] + 1;
            colno[1] = bb.n_op_in_block + 1;
            row[0] = 1;
            row[1] = -1;
            int latency = get_latency(ot2rtid, hin, opid);
            if (!add_constraintex(lp, 2, row, colno, LE, 0)) ret = -1;
        }
    }

    // Resource constraints
    vector<vector<int>> topos;
    if (topology_sort(g, *hin, topos) < 0) ret = -1;

    for (int otid = 0; otid < n_op_type; otid++) {
        // ignore those operations needless to schdule
        OpCategory opcate = hin->op_types[otid];
        if (!hin->need_bind(opcate)) continue;

        auto &topo = topos[otid];
        int k = insts[otid];
        if (k <= 0) {
            ret = -1;
            break;
        }

        // get resource's latency
        int rtid = ot2rtid[otid];
        const auto &rt = hin->resource_types[rtid];
        int latency;
        if (rt.is_sequential) {
            if (rt.is_pipelined)
                latency = 1;
            else
                latency = rt.latency + 1; // still has delay!
        } else {
            latency = 1;  // otherwise combinational ops will conflict
        }

        // add constraints on interval of k
        for (int i = k; i < topo.size(); i++) {
            // x_{i+k} - x_i >= Latency
            colno[0] = op2idx[topo[i]] + 1;
            colno[1] = op2idx[topo[i - k]] + 1;
            row[0] = 1;
            row[1] = -1;
#ifdef DEBUG_HLS_SCHEDULE_SDC
            cerr << "op[" << topo[i] << "] - op[" << topo[i - k]
                 << "] >= " << latency << std::endl;
#endif
            if (!add_constraintex(lp, 2, row, colno, GE, latency)) {
                cerr << "Error on adding resource constraints" << endl;
                ret = -1;
            }
        }
    }

    // Chaining constraints (ignored in basic version)

    // clean up
    delete[] colno;
    delete[] row;
    return ret;
}

}  // namespace hls

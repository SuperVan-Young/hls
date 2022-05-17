#include "graph.h"

namespace hls {

// build an induced graph on a basic block
// Considering all dependencies except for back edges from other basic blocks.
AdjacentList build_induced_graph(int bbid, const HLSInput &hin) {
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

// Give a topology sort for non-empty DAG and write to out.
// Returns 0 on success, -1 on errors (loops).
int topology_sort(AdjacentList g, vector<int> &out) {
    out.empty();

    queue<int> q_ready;

    for (auto it = g.begin(); it != g.end(); it++) {
        if (it->second.first == 0) {  // in degree == 0
            q_ready.push(it->first);
        }
    }

    while (!q_ready.empty()) {
        auto v = q_ready.front();
        q_ready.pop();
        out.push_back(v);
        auto node = g[v];
        for (auto u: node.second) {  // out vertex
            if ((--g[u].first) == 0) {  // in degree
                q_ready.push(u);
            }
        }
    }

    if (out.size() != g.size())
        return -1;
    return 0;
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

};  // namespace hls
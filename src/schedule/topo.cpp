#include <queue>
#include <vector>

#include "topo.h"

namespace hls {

// Collect CDFG to form a DAG of TopoNodes
std::vector<TopoNode> gather_CDFG(const std::vector<Operation> &ops) {
    std::vector<TopoNode> g(ops.size());

    for (int i = 0; i < ops.size(); i++) {
        g[i].id = i;
        const Operation &op = ops[i];
        for (int j = 0; j < op.n_inputs; j++) {
            int v = op.inputs[j];
            if (v != -1) {
                g[i].in++;
                g[v].out.push_back(i);
            }
        }
    }

    return g;
}

// Return a topology sort of DAG g to res
// This may modify g.
// Returns 0 on success, -1 if it detects rings.
int topology_sort(std::vector<TopoNode> &g, std::vector<int> &res) {
    std::priority_queue<TopoNode, std::vector<TopoNode>, std::greater<TopoNode>>
        q;  // priority queue for toponodes of in_degree=0

    // initialize q
    for (int i = 0; i < g.size(); i++) {
        TopoNode &node = g[i];
        if (node.in == 0) q.push(node);
    }
    // initialize res
    res.empty();

    while (!q.empty()) {
        TopoNode node = q.top();
        q.pop();
        res.push_back(node.id);
        for (auto v: node.out) {
            if ((--g[v].in) == 0)
                q.push(g[v]);
        }
    }

    if (res.size() != g.size())
        return -1;
    return 0;
}

};  // namespace hls
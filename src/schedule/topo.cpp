#include "topo.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <vector>

namespace hls {

// Collect CDFG to form a DAG of TopoNodes
// You must use the complete operation array
std::vector<TopoNode> gather_CDFG(const HLSInput &hin) {
    const std::vector<Operation> &ops = hin.operations;
    std::vector<TopoNode> g(ops.size());

    for (int i = 0; i < ops.size(); i++) {
        const Operation &op = ops[i];
        g[i].id = op.opid;
        g[i].bbid = op.bbid;
        for (int j = 0; j < op.n_inputs; j++) {
            int v = op.inputs[j];
            if (v != -1) {
                g[i].in++;
                g[v].out.push_back(i);
            }
        }
        // Operations of PHI, BR, ALLOCA will be scheduled to -1
        // Clear their in_degree to assure priority and cut off potential loops
        OpCategory otype = hin.op_types[ops[i].optype];
        if (otype == OP_ALLOCA || otype == OP_BRANCH || otype == OP_PHI)
            g[i].in = 0;
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
        for (auto v : node.out) {
            if ((--g[v].in) == 0) q.push(g[v]);
        }
    }

    if (res.size() != g.size()) return -1;
    return 0;
}

// Return an ordering of basic blocks.
// Considering performances, we schedule one basic block at a time.
// We first schedule basic blocks with operation of higher priority,
// that is, these ops are at the beginning of toposort.
std::vector<int> order_bb(const HLSInput &hin,
                          const std::vector<int> &toposort) {
    // pair = (first operation, block number), it's easier to sort
    std::vector<std::pair<int, int>> res(hin.n_block);
    for (int i = 0; i < hin.n_block; i++) {
        res[i] = std::make_pair(-1, i);
    }

    for (int i = 0; i < toposort.size(); i++) {
        int opid = toposort[i];
        int bbid = hin.operations[opid].bbid;
        OpCategory otype = hin.op_types[hin.operations[opid].optype];
        if (otype == OP_ALLOCA || otype == OP_BRANCH || otype == OP_PHI)
            continue;
        res[i] = std::make_pair(opid, bbid);
    }

    std::sort(res.begin(), res.end());

    std::vector<int> r(hin.n_block);
    for (int i = 0; i < hin.n_block; i++) r[i] = res[i].second;
    return r;
}

// Return a basic block's operations, return it's sub topology sort
std::vector<int> subgraph_bb(const std::vector<int> &ops,
                             const std::vector<int> &toposort) {
    std::vector<int> res;
    for (auto e : toposort) {
        for (int j = 0; j < ops.size(); j++) {
            if (e == ops[j]) {
                res.push_back(e);
                break;
            }
        }
    }
    if (res.size() != ops.size()) {
        std::cerr << "Error: subgraph_bb" << std::endl;
    }
    return res;
}

};  // namespace hls
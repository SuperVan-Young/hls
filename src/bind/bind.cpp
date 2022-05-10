#include "bind.h"

// #define DEBUG

namespace hls {

// Check if an operation needs binding
bool ConflictGraph::need_binding(int opid, const HLSInput &hin) {
    const auto &op = hin.operations[opid];
    OpCategory op_cate = hin.get_op_cate(&op);
    if (op_cate != OP_ARITHM && op_cate != OP_BOOL && op_cate != OP_COMPARE)
        return false;
    return true;
}

// Check if two operations have conflict.
// Returns true on conflict, false on no conflict.
bool ConflictGraph::check_conflict(int opid1, int opid2, const HLSInput &hin,
                                   const HLSOutput &hout) {
    const Operation &op1 = hin.operations[opid1];
    const Operation &op2 = hin.operations[opid2];

    // Operations of the same type?
    if (op1.optype != op2.optype) return false;

    // Operation needs to be binded?
    if (!need_binding(opid1, hin)) return false;

    // Execution overlaps?
    int optype = op1.optype;
    int rstype = hout.op2rs[optype];
    const ResourceType &rs = hin.resource_types[rstype];
    int early = std::min(hout.scheds[opid1].cycle, hout.scheds[opid2].cycle);
    int late = std::max(hout.scheds[opid1].cycle, hout.scheds[opid2].cycle);
    if (rs.is_pipelined) {
        if (early == late) return true;
    } else {
        if (early + rs.latency > late) return true;
    }
    return false;
}

void ConflictGraph::add_conflict(int opid1, int opid2) {
    edges[opid1].push_back(opid2);
    edges[opid2].push_back(opid1);
}

// Greedily add a color for op, without conflict with its colored neighboors.
// You decide when to add color for op (according to your PEO)
// return non-negative int on success, -1 on error.
int ConflictGraph::add_color(int op) {
    // gather colors that neighboors used
    vector<int> used;
    for (auto e : edges[op]) {
        if (colors[e] != -1) used.push_back(colors[e]);
    }

    // find the first color op's neighboors didn't use
    int res = 0;
    for (int i = 0; i < used.size(); i++) {
        int c = used[i];
        if (res < c)
            break;
        res++;
    }

    // coloring current operation
    colors[op] = res;
    max_color = std::max(res, max_color);
    return res;
}

// Sorting interval graph with left edge algorithm
// In module binding, left edge is just its start cycle
// Returns: vector of pairs, in which pair = (cycle, opid)
vector<pair<int, int>> sort_interval_graph(const HLSOutput &hout) {
    vector<pair<int, int>> res(hout.n_operation);
    for (int i = 0; i < res.size(); i++) {
        res[i] = std::make_pair(hout.scheds[i].cycle, i);
    }
    sort(res.begin(), res.end());
    return res;
}

// Bind scheduled operations to resource instances
// return 0 on success, -1 on errors
int bind(const HLSInput &hls_input, HLSOutput &hls_output) {
    // build a conflict graph
    ConflictGraph conf_graph;
    conf_graph.setup(hls_input.n_operation);
    for (int i = 0; i < hls_input.n_operation; i++) {
        for (int j = i + 1; j < hls_input.n_operation; j++) {
            bool is_conf =
                conf_graph.check_conflict(i, j, hls_input, hls_output);
            if (is_conf) {
                conf_graph.add_conflict(i, j);
            }
        }
    }

    // debug: print conf graph
#ifdef DEBUG
    for (int i = 0; i < conf_graph.n_vertex; i++) {
        std::cerr << i << ": ";
        for (auto n : conf_graph.edges[i]) std::cerr << n << ' ';
        std::cerr << std::endl;
    }
#endif

    // Get a PEO
    auto peo = sort_interval_graph(hls_output);

    // binding operations
    for (auto node : peo) {
        int opid = node.second;
        if (conf_graph.need_binding(opid, hls_input))
            conf_graph.add_color(opid);
    }

    // debug: print coloring result
#ifdef DEBUG
    std::cout << "Coloring results:\n";
    for (auto c : conf_graph.colors) {
        std::cout << c << ' ';
    }
    std::cout << std::endl;
#endif

    // Write back the result
    for (int i = 0; i < hls_input.n_operation; i++) {
        hls_output.scheds[i].inst = conf_graph.colors[i];
    }
    return 0;
}

};  // namespace hls
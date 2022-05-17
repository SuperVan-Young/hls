#include "base.h"

// #define DEBUG

namespace hls {

// Check if two operations have conflict.
// Returns true on conflict, false on no conflict.
bool BaseBinder::check_conflict(int opid1, int opid2) {
    const Operation &op1 = hin->operations[opid1];
    const Operation &op2 = hin->operations[opid2];

    // Operations of the same type?
    if (op1.optype != op2.optype) return false;

    // Operation needs to be binded?
    if (!hin->need_bind(hin->get_opcate(opid1))) return false;

    // Execution overlaps?
    int optype = op1.optype;
    int rstype = hout->ot2rtid[optype];
    const ResourceType &rs = hin->resource_types[rstype];
    int early = std::min(hout->scheds[opid1], hout->scheds[opid2]);
    int late = std::max(hout->scheds[opid1], hout->scheds[opid2]);
    if (rs.is_pipelined) {
        if (early == late) return true;
    } else {
        if (late - early < rs.latency + 1) return true;
    }
    return false;
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
        if (res < c) break;
        res++;
    }

    // coloring current operation
    colors[op] = res;
    max_color = std::max(res, max_color);
    return res;
}

// Bind scheduled operations to resource instances
// return 0 on success, -1 on errors
int BaseBinder::bind() {
    // build a conflict graph
    ConflictGraph conf_graph(n_operation);
    for (int i = 0; i < n_operation; i++) {
        for (int j = i + 1; j < n_operation; j++) {
            if (check_conflict(i, j)) {
                conf_graph.add_conflict(i, j);
            }
        }
    }

    // Get a PEO
    auto peo = sort_interval_graph(*hout);

    // binding operations
    for (auto node : peo) {
        int opid = node.second;
        if (hin->need_bind(hin->get_opcate(opid))) conf_graph.add_color(opid);
    }

    // write color to binds
    vector<int> cnt_rtype(hin->n_resource_type, 0);
    vector<int> op2rbase(n_op_type, 0);  // optype -> smallest resource id
    for (int optype = 0; optype < n_op_type; optype++) {
        int rtid = hout->ot2rtid[optype];
        if (rtid != -1) {
            op2rbase[optype] = cnt_rtype[rtid];
            cnt_rtype[rtid] += hout->insts[optype];
        }
    }
    for (int i = 0; i < n_operation; i++) {
        if (hin->need_bind(hin->get_opcate(i))) {
            int color = conf_graph.colors[i];
            int optype = hin->operations[i].optype;
            binds[i] = op2rbase[optype] + color;
        } else {
            binds[i] = -1;
        }
    }

    return 0;
}

void BaseBinder::copyout(HLSOutput &hout) {
    for (int i = 0; i < n_operation; i++) {
        hout.binds[i] = binds[i];
    }
}

bool RBinder::check_conflict(int opid1, int opid2) {
    const Operation &op1 = hin->operations[opid1];
    const Operation &op2 = hin->operations[opid2];

    // Operation needs to be binded?
    if (!hin->need_bind(hin->get_opcate(opid1))) return false;
    if (!hin->need_bind(hin->get_opcate(opid2))) return false;

    // Operation shares resource type?
    if (hout->ot2rtid[op1.optype] != hout->ot2rtid[op2.optype]) return false;

    // Execution overlaps?
    int optype = op1.optype;
    int rstype = hout->ot2rtid[optype];
    const ResourceType &rs = hin->resource_types[rstype];
    int early = std::min(hout->scheds[opid1], hout->scheds[opid2]);
    int late = std::max(hout->scheds[opid1], hout->scheds[opid2]);
    if (rs.is_pipelined) {
        if (early == late) return true;
    } else {
        if (late - early < rs.latency + 1) return true;
    }
    return false;
}

int RBinder::bind() {
    // build a conflict graph
    ConflictGraph conf_graph(n_operation);
    for (int i = 0; i < n_operation; i++) {
        for (int j = i + 1; j < n_operation; j++) {
            if (check_conflict(i, j)) {
                conf_graph.add_conflict(i, j);
            }
        }
    }

    // Get a PEO
    auto peo = sort_interval_graph(*hout);

    // binding operations
    for (auto node : peo) {
        int opid = node.second;
        if (hin->need_bind(hin->get_opcate(opid))) conf_graph.add_color(opid);
    }

    // write color to binds
    for (int i = 0; i < n_operation; i++) {
        if (hin->need_bind(hin->get_opcate(i))) {
            int color = conf_graph.colors[i];
            binds[i] = color;
        } else {
            binds[i] = -1;
        }
    }

    return 0;
}

};  // namespace hls
#include "base.h"

namespace hls {

// Give an order to schedule basic block
vector<int> BaseScheduler::sort_basic_block() {
    vector<int> order;
    vector<bool> finished_block(n_block, false);
    vector<bool> ready_list(n_operation, false);
    queue<int> bfs;

    // find the entry block
    for (int bbid = 0; bbid < n_block; bbid++) {
        const BasicBlock &bb = hin->blocks[bbid];
        if (bb.n_pred == 0) {
            bfs.push(bbid);
            break;
        }
    }

    // bfs basic blocks until all operations are finished
    while (!bfs.empty()) {
        int bbid = bfs.front();
        bfs.pop();

        // DO NOT rerun
        if (finished_block[bbid]) continue;

        const BasicBlock &bb = hin->blocks[bbid];
        if (is_basic_block_ready(bbid, *hin, ready_list)) {
            // finish ops in basic block
            for (auto v : bb.ops) ready_list[v] = true;
            finished_block[bbid] = true;
            order.push_back(bbid);

            // search unfinished successors
            for (auto succ : bb.succs) {
                bfs.push(succ);
            }
        }
    }

    return order;
}

// check if the basic block is ready to schedule
bool is_basic_block_ready(int bbid, const HLSInput &hin,
                          const vector<bool> &ready_list) {
    const auto &bb = hin.blocks[bbid];
    // construct a set for quicker searching
    set<int> op_in_block(bb.ops.begin(), bb.ops.end());

    for (auto opid : bb.ops) {
        const auto &op = hin.operations[opid];
        // PHI nodes could be scheduled unconditionally
        if (hin.get_opcate(opid) == OP_PHI) continue;
        for (auto in : op.inputs) {
            if (op_in_block.count(in)) continue;  // ignore op in block
            if (!ready_list[in]) return false;    // not ready!
        }
    }
    return true;
}

// Schedule a block from cycle 0 and write to res
// return max time step on success, -1 on errors
int BaseScheduler::schedule_block(int bbid, map<int, int> &res) {
    const auto &bb = hin->blocks[bbid];
    res.empty();

    // build induced graph
    AdjacentList list = build_induced_graph(bbid, *hin);

    // schedule according to topology sort
    vector<int> topo;
    if (topology_sort(list, topo) != 0) {
        std::cerr << "Error in topology sort!" << std::endl;
        return -1;
    }

    int l = 0;
    for (auto opid : topo) {
        const auto &op = hin->operations[opid];
        OpCategory opcate = hin->get_opcate(opid);
        if (opcate == OP_ALLOCA || opcate == OP_BRANCH || opcate == OP_PHI)
            res.insert(std::make_pair(opid, -1));
        else
            res.insert(std::make_pair(opid, l));

        // update l
        int rtid = ot2rtid[op.optype];
        int latency = hin->resource_types[rtid].latency;
        l += latency + 1;  // result must have been ready by now
    }
    return l;
}

// Schedule all operations
// Returns 0 on success, -1 on errors.
int BaseScheduler::schedule() {
    vector<int> order = sort_basic_block();
    int start = 1;
    int lasting;
    if (order.size() != n_block) {
        std::cerr << "Error: Base Scheduler sort blocks " << std::endl;
        for (auto bbid : order) std::cerr << bbid << ' ';
        std::cerr << std::endl;
        return -1;
    }

    for (auto bbid : order) {
        map<int, int> bb_sched;

        if ((lasting = schedule_block(bbid, bb_sched)) < 0) {
            std::cerr << "Error: Base Scheduler scheduling" << std::endl;
            return -1;
        }

        for (auto it = bb_sched.begin(); it != bb_sched.end(); it++) {
            int opid = it->first;
            int cycle = it->second;
            scheds[opid] = (cycle == -1 ? 0 : start) + cycle;
        }
        start += lasting;
    }
    return 0;
}

void BaseScheduler::copyout(HLSOutput &hout) {
    for (int i = 0; i < n_operation; i++) {
        hout.scheds[i] = scheds[i];
    }
}

}  // namespace hls
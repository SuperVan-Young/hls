#include "list.h"

#include <algorithm>
#include <iostream>
#include <vector>

using std::vector;

namespace hls {

// setup global variables
int setup_listnodes(const HLSInput &hin) {
    // initialize global variables
    Currentbbid = -1;
    CntScheduled = 0;
    ListNodes = vector<ListNode>(hin.n_operation);

    // construct CDFG
    for (int i = 0; i < hin.n_operation; i++) {
        ListNode &node = ListNodes[i];
        const Operation *op = &hin.operations[i];
        node.op = op;

        OpCategory op_cate = hin.get_op_cate(&hin.operations[i]);
        // traverse all edges
        for (int j = 0; j < op->n_inputs; j++) {
            int opid = op->inputs[j];
            ListNodes[opid].outs.push_back(i);
        }
        node.in = op->n_inputs;
    }

    // Schedule operations starting at cycle -1
    for (int i = 0; i < hin.n_operation; i++) {
        ListNode &node = ListNodes[i];
        OpCategory op_cate = hin.get_op_cate(&hin.operations[i]);
        if (op_cate == OP_ALLOCA || op_cate == OP_BRANCH || op_cate == OP_PHI) {
            node.in = 0;
            node.cycle = -1;
            for (auto j: node.outs) {
                ListNodes[j].in--;
            }
            CntScheduled++;
        }
    }

    return 0;
}

int setup_listinsts(const HLSInput &hin, const HLSOutput &hout) {
    ListInsts = vector<ListInst>(hout.n_op_type);

    for (int i = 0; i < hin.n_op_type; i++) {
        int rtype = hout.op2rs[i];
        ListInst &listinst = ListInsts[i];
        if (rtype != -1) {
            // for unbinded ops, rtype remains nullptr
            listinst.rtype = &hin.resource_types[rtype];

            // count resource instances
            auto &insts = hout.insts[rtype];
            for (auto &inst : insts) {
                if (inst.op_type == i) {
                    listinst.max_insts++;
                }
            }
            // For load and store, they could have unlimited resource insts
            OpCategory op_cate = hin.op_types[i];
            if (op_cate == OP_LOAD || op_cate == OP_STORE) {
                listinst.max_insts = 9999999;
            }
        }
    }

    // every instances are available at first
    for (int i = 0; i < hin.n_op_type; i++) {
        ListInsts[i].avail_insts = ListInsts[i].max_insts;
    }
    return 0;
}

bool cmp_listnode(const ListNode *a, const ListNode *b) { return (*a) < (*b); }

// List Scheduling.
// write the result to scheds
// Returns 0 on success, -1 on errors.
int schedule_list(const HLSInput &hin, const HLSOutput &hout,
                  std::vector<int> &scheds) {
    int l = 1;
    int max_steps = 9999;

    setup_listnodes(hin);
    setup_listinsts(hin, hout);

    // collect first finished operations
    ready_list.empty();
    for (auto &node : ListNodes) {
        if (node.in == 0 && node.cycle == 0) ready_list.push_back(&node);
    }

    // start list scheduling
    while (CntScheduled < hin.n_operation) {
        if (l >= max_steps) {
            // debugging on endless loops
            std::cerr << "Endless Loop in list scheduling!" << std::endl;
            for (int i = 0; i < hin.n_operation; i++) {
                std::cerr << ListNodes[i].cycle << ' ';
            }
            std::cerr << std::endl;
            return -1;
        }
        sort(ready_list.begin(), ready_list.end(), cmp_listnode);
        for (auto it = ready_list.begin(); it != ready_list.end();) {
            auto node = *it;
            int op_type = node->op->optype;

            // debug
            // std::cerr << "debug: " << op_type << ' ' << ListInsts.size() << std::endl;

            auto &inst = ListInsts[op_type];
            if (inst.is_busy()) {
                it++;
                continue;
            } else {
                inst.add_exec(node->op->opid, l);
                it = ready_list.erase(it);
            }
        }
        // update ready lists
        l++;
        for (auto &inst : ListInsts) inst.exec(l);
    }

    // write result back to scheds
    scheds.empty();
    for (int i = 0; i < hin.n_operation; i++) {
        scheds.push_back(ListNodes[i].cycle);
    }
    return 0;
}

// add an executing operation to the instance
void ListInst::add_exec(int op, int cycle) {
    if (!rtype) return;
    ListNodes[op].cycle = cycle;
    execs.push(std::make_pair(op, cycle));
    avail_insts--;
    Currentbbid = ListNodes[op].op->bbid;
}

// update insts every cycle
void ListInst::exec(int cycle) {
    if (!rtype) return;
    // for pipelined resources, all insts become available next cycle
    if (rtype->is_pipelined) avail_insts = max_insts;
    while (!execs.empty()) {
        auto top = execs.front();
        int start = top.second;
        if (start + rtype->latency + 1 == cycle) {
            // we don't consider chaining here
            execs.pop();
            CntScheduled++;
            // Add ready ops
            auto &node = ListNodes[top.first];
            for (auto out : node.outs) {
                auto &outNode = ListNodes[out];
                if ((--outNode.in) == 0) {
                    ready_list.push_back(&outNode);
                }
            }
            if (!rtype->is_pipelined) avail_insts++;
        } else
            break;  // not the right cycle to finish these jobs
    }
}

}  // namespace hls

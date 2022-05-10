#ifndef HLS_SCHEDULE_LIST_H
#define HLS_SCHEDULE_LIST_H

#include <queue>
#include <vector>

#include "io.h"

using std::pair;
using std::queue;
using std::vector;

namespace hls {

// current basic block index
// Operations in the same bb gets scheduled first.
static int Currentbbid = -1;

// How many operations have been scheduled
static int CntScheduled = 0;

// Nodes to record operation informations
class ListNode {
   public:
    const Operation *op = nullptr;
    int in = 0;
    vector<int> outs;
    int cycle = 0;  // scheduled to which cycle?

    bool operator<(const ListNode &x) const {
        const Operation *a = this->op;
        const Operation *b = x.op;
        if (a->bbid == Currentbbid || b->bbid == Currentbbid) {
            if (a->bbid == b->bbid)
                return a->opid < b->opid;  // smaller op id first
            else
                return a->opid == Currentbbid;  // current bb first
        } else {
            if (a->bbid != b->bbid)
                return (a->bbid < b->bbid);  // smaller bbid comes first
            else
                return a->opid < b->opid;  // smaller opid first
        }
    }
};
static vector<ListNode> ListNodes;
static vector<ListNode *> ready_list;

// Record resources instances information
class ListInst {
   public:
    int avail_insts = 0;
    int max_insts = 0;
    const ResourceType *rtype = nullptr;
    queue<pair<int, int>> execs;  // ongoing operation and its start cycle

    bool is_busy() { return avail_insts == 0; }

    void add_exec(int op, int cycle);

    void exec(int cycle);
};

static vector<ListInst> ListInsts;  // length = n_op_type

int setup_listnodes(const HLSInput &hin);

int setup_listinsts(const HLSInput &hin, const HLSOutput &hout);

int schedule_list(const HLSInput &hin, const HLSOutput &hout,
                  std::vector<int> &scheds);
};  // namespace hls

#endif
#ifndef HLS_SCHEDULE_BASE_H
#define HLS_SCHEDULE_BASE_H

#include <queue>
#include <set>
#include <vector>

#include "io.h"
#include "graph.h"

using std::map;
using std::queue;
using std::set;
using std::vector;

namespace hls {

// Basic scheduler
// Schedule one basic block at a time and give a worst linear scheduling.
class BaseScheduler {
   protected:
    int n_block;
    int n_operation;
    int n_op_type;
    int n_resource_type;
    const HLSInput *hin;
    vector<int> ot2rtid;
    vector<int> insts;
    vector<int> rinsts;
    vector<int> scheds;

   public:
    BaseScheduler(const HLSInput &hin, const HLSOutput &hout) {
        n_block = hin.n_block;
        n_operation = hin.n_operation;
        n_op_type = hin.n_op_type;
        n_resource_type = hin.n_resource_type;
        this->hin = &hin;
        ot2rtid = vector<int>(hout.ot2rtid);
        insts = vector<int>(hout.insts);
        rinsts = vector<int>(hout.rinsts);
        scheds.resize(n_operation, 0);
    }

    vector<int> sort_basic_block();

    virtual int schedule_block(int bbid, map<int, int> &res);

    int schedule();

    void copyout(HLSOutput &hout);
};

bool is_basic_block_ready(int bbid, const HLSInput &hin,
                          const vector<bool> &ready_list);
};  // namespace hls

#endif
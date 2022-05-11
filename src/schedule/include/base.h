#ifndef HLS_SCHEDULE_BASE_H
#define HLS_SCHEDULE_BASE_H

#include <queue>
#include <vector>
#include <set>

#include "io.h"

using std::map;
using std::queue;
using std::vector;
using std::set;

namespace hls {

typedef pair<int, vector<int>> AdjacentNode;
typedef map<int, AdjacentNode> AdjacentList;

// Basic scheduler
// Schedule one basic block at a time and give a worst linear scheduling.
class BaseScheduler {
   public:
    int n_block;
    int n_operation;
    int n_op_type;
    const HLSInput *hin;
    vector<int> ot2rtid;
    vector<int> insts;
    vector<int> scheds;

    BaseScheduler(const HLSInput &hin, const HLSOutput &hout) {
        n_block = hin.n_block;
        n_operation = hin.n_operation;
        n_op_type = hin.n_op_type;
        this->hin = &hin;
        ot2rtid = vector<int>(hout.ot2rtid);
        insts = vector<int>(hout.insts);
        scheds.resize(n_operation, 0);
    }

    vector<int> sort_basic_block();

    int schedule_block(int bbid, map<int, int> &res);

    int schedule();

    void copyout(HLSOutput &hout);
};

bool is_basic_block_ready(int bbid, const HLSInput &hin,
                          const vector<bool> &ready_list);

AdjacentList build_induced_graph(int bbid, const HLSInput &hin);

int topology_sort(AdjacentList g, vector<int> &out);

};  // namespace hls

#endif
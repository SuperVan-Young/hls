#ifndef HLS_SCHEDULE_SDC_H
#define HLS_SCHEDULE_SDC_H

#include "base.h"
#include "io.h"
#include "lp_lib.h"

namespace hls {

class SDCScheduler : public BaseScheduler {
   public:
    SDCScheduler(const HLSInput &hin, const HLSOutput &hout)
        : BaseScheduler(hin, hout) {}

    int schedule_block(int bbid, map<int, int> &res);

    int add_constraints(int bbid, lprec *lp);
};

map<int, int> build_op2idx(const BasicBlock &bb);

}  // namespace hls

#endif
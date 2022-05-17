#ifndef HLS_SCHEDULE_SDC_H
#define HLS_SCHEDULE_SDC_H

#include "base.h"
#include "io.h"
#include "lp_lib.h"

namespace hls {

class SDCScheduler : public BaseScheduler {
   public:
   bool rlimit = false;  // add resource constraints or not
    SDCScheduler(const HLSInput &hin, const HLSOutput &hout, bool rlimit)
        : BaseScheduler(hin, hout) {
        this->rlimit = rlimit;
    }

    int schedule_block(int bbid, map<int, int> &res);

    int add_constraints(int bbid, lprec *lp);
};

}  // namespace hls

#endif
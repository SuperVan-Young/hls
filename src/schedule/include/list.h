#ifndef HLS_SCHEDULE_LIST_H
#define HLS_SCHEDULE_LIST_H

#include "io.h"
#include "topo.h"

namespace hls {

// Priority functions for list scheduling
enum SchedPriorFuncType {
    SPFT_LATENCY,
};

int schedule_list(HLSInput &hin, HLSOutput &hout,
                  std::vector<OperationSched> &scheds,
                  SchedPriorFuncType functype);
};

#endif
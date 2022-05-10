#include <iostream>

#include "algo.h"
#include "io.h"
#include "list.h"

namespace hls {

int schedule(const HLSInput &hls_input, HLSOutput &hls_output) {
    std::vector<int> scheds;
    if (schedule_list(hls_input, hls_output, scheds) == -1) {
        return -1;
    }
    for (int i = 0; i < hls_output.n_operation; i++)
        hls_output.scheds[i].cycle = scheds[i];
    return 0;
}

};  // namespace hls

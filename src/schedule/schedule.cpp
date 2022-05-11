#include <iostream>

#include "algo.h"
#include "io.h"
#include "list.h"
#include "base.h"

namespace hls {

int schedule(const HLSInput &hls_input, HLSOutput &hls_output) {
    auto scheduler = BaseScheduler(hls_input, hls_output);
    if (scheduler.schedule() < 0)
        return -1;
    scheduler.copyout(hls_output);
    return 0;
}

};  // namespace hls

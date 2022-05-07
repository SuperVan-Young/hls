// HLS Algorithms
// We tackle this problem step by step
#ifndef HLS_ALGO_H
#define HLS_ALGO_H

#include "io.h"

namespace hls {

int allocate(const HLSInput &hls_input, HLSOutput &hls_output);

int schedule(const HLSInput &hls_input, HLSOutput &hls_output);

int bind(const HLSInput &hls_input, HLSOutput &hls_output);

};  // namespace hls

#endif
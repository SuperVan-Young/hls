#ifndef HLS_ALLOCATE_PERF_H
#define HLS_ALLOCATE_PERF_H

#include <vector>

#include "io.h"

using std::vector;

namespace hls {

// Allocate each operation w.r.t. its expected latency
class PerfAllocator {
   private:
    const HLSInput *hin;
    int n_op_type = 0;
    vector<int> opid2rtid;  // length = n_op_type
    vector<int> insts;      // length = n_op_type

   public:
    PerfAllocator(const HLSInput &hin) {
        this->hin = &hin;
        this->n_op_type = hin.n_op_type;
        opid2rtid.resize(n_op_type, -1);
        insts.resize(n_op_type, 0);
    }
}

}  // namespace hls

#endif
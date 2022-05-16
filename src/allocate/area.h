#ifndef HLS_ALLOCATE_AREA_H
#define HLS_ALLOCATE_AREA_H

#include <vector>

#include "io.h"

using std::vector;

namespace hls {

// Area Allocator that minimize area cost
// Allocate each operation type with an resource type of minimum area
// and only one instance.
class AreaAllocator {
   protected:
    const HLSInput *hin;
    int n_op_type = 0;
    vector<int> ot2rtid;  // length = n_op_type
    vector<int> insts;    // length = n_op_type

   public:
    AreaAllocator(const HLSInput &hin) {
        this->hin = &hin;
        this->n_op_type = hin.n_op_type;
        ot2rtid.resize(n_op_type, -1);
        insts.resize(n_op_type, 0);
    }
    void allocate_type();
    void allocate_inst();
    bool validate();
    void copyout(HLSOutput &hout);
    void print(bool verbose = false) const;
};

}  // namespace hls

#endif
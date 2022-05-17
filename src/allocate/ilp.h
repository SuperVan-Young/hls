#ifndef HLS_ALLOCATE_ILP_H
#define HLS_ALLOCATE_ILP_H

#include "io.h"
#include "lp_lib.h"
#include <limits>

namespace hls {

class ILPAllocator {
   private:
    const HLSInput *hin;
    vector<bool> rtypes;  // the chosen resource types to use
    vector<int> ot2rtid;  // optype binds to which resource type?
    vector<vector<int>> ot2comprt;  // optype -> compatible rtype
    const float theta_pipeline = 2.0;

   public:
    ILPAllocator(const HLSInput &hin) {
        this->hin = &hin;
        this->rtypes.resize(hin.n_resource_type, false);
        this->ot2rtid.resize(hin.n_op_type, -1);

        this->ot2comprt.resize(hin.n_resource_type);
        for (const auto &rt : hin.resource_types)
            for (auto ot : rt.comp_ops)
                ot2comprt[ot].push_back(rt.rtid);
    }

    // Allocate resource types to cover all operation types
    // Returns 0 on success, -1 on errors.
    int allocate_resource_type();

    // Allocate each operation type with a specific resource type
    // Returns 0 on success, -1 on errors.
    int allocate_operation_type();

    // Set upper bounds for resource insts if binding exceeds area limit
    // Returns 0 on success, -1 on errors.
    int allocate_insts_bound(vector<int> &rinsts);
};

}  // namespace hls

#endif
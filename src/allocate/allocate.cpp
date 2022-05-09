#include <iostream>

#include "algo.h"
#include "io.h"

namespace hls {

/* Allocation Utility Functions */

enum AllocUtilFuncType {
    AUFT_MINIMUM_AREA,
    AUFT_MINIMUM_LATENCY,
    AUFT_MINIMUM_AREA_LAT_PROD
};

typedef float (*PtrAllocUtilFunc)(const ResourceType &, const HLSInput &);

float auf_area(const ResourceType &r, const HLSInput &h) { return (float)r.area; }

float auf_latency(const ResourceType &r, const HLSInput &h) {
    return (float)(r.latency + r.delay / h.target_cp);
}

float auf_area_lat_prod(const ResourceType &r, const HLSInput &h) {
    return (float)((r.latency + r.delay / h.target_cp) * r.area);
}

// Allocate each operation with a resource type.
// Choose the best resource type with minimun unility function's result.
// Returns 0 on success, -1 on error.
int allocate_type(const HLSInput &hin, HLSOutput &hout,
                  AllocUtilFuncType func_type) {
    std::vector<float> utils(hin.n_op_type, 10000000.0);
    std::vector<int> sched_types(hin.n_op_type, -1);
    PtrAllocUtilFunc util_func = nullptr;

    // Choose a utility function
    switch (func_type) {
        case AUFT_MINIMUM_AREA:
            util_func = auf_area;
            break;
        case AUFT_MINIMUM_LATENCY:
            util_func = auf_latency;
            break;
        case AUFT_MINIMUM_AREA_LAT_PROD:
            util_func = auf_area_lat_prod;
            break;
    }

    // Scan through resource_types and allocate resource type to operation type
    for (int i = 0; i < hin.n_resource_type; i++) {
        const ResourceType &r = hin.resource_types[i];
        for (auto cop : r.comp_ops) {
            float tmp_util = util_func(r, hin);
            if (tmp_util < utils[cop]) {
                utils[cop] = tmp_util;
                sched_types[cop] = i;
            }
        }
    }

    // Validate the result.
    // OpType 1~5 must have no allocation (nor scheduling),
    // while OpType 6~8 must have a positive binding
    for (int i = 0; i < hin.n_op_type; i++) {
        OpCategory op_type = hin.op_types[i];
        int sched_type = sched_types[i];
        if (op_type >= OP_ARITHM && op_type <= OP_COMPARE) {
            if (sched_type == -1) return -1;
        } else {
            if (sched_type != -1) return -1;
        }
    }

    // write back to hls output
    for (int i = 0; i < hin.n_op_type; i++) {
        hout.op2rs[i] = sched_types[i];
    }
    return 0;
}

// Allocate each resource type with certain number of instances.
// Don't care if binding algorithm actually uses them.
// Return 0 on success, -1 on errors.
int allocate_inst(const HLSInput &hin, HLSOutput &hout) {
    std::vector<std::vector<ResourceInst>> insts(hin.n_resource_type);

    // A naive solution: allocate one instance for each operation
    // Combined with minimum-area utility function, this must lead to a valid
    // allocation, otherwise the area limit is too small.
    for (int i = 0; i < hin.n_op_type; i++) {
        int rtype = hout.op2rs[i];
        if (rtype != -1) {
            ResourceInst inst(rtype, i);
            insts[rtype].push_back(inst);
        }
    }

    // TODO: allocate remaining areas.

    // Validate the result
    // Should not violate area limit.
    int total_area = 0;
    for (int i = 0; i < hin.n_resource_type; i++) {
        int area = hin.resource_types[i].area;
        int num = insts[i].size();
        total_area += area * num;
    }
    if (total_area > hin.area_limit)    
        return -1;

    // write insts back to hout
    for (int i = 0; i < hin.n_resource_type; i++) {
        for (auto j: insts[i])
            hout.insts[i].push_back(j);
    }
    return 0;

}

// Allocate each operation with a resource type, and set limit to resource
// instances. Scheduling may only partially use allocation result.
// Returns 0 on success, -1 on error.
int allocate(const HLSInput &hls_input, HLSOutput &hls_output) {
    if (allocate_type(hls_input, hls_output, AUFT_MINIMUM_AREA) != 0) {
        std::cerr << "Error: allocate_type" << std::endl;
        return -1;
    }
    if (allocate_inst(hls_input, hls_output) != 0) {
        std::cerr << "Error: allocate_inst" << std::endl;
        return -1;
    }
    return 0;
}

};  // namespace hls
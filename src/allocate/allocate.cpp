#include <iostream>

#include "algo.h"
#include "io.h"
#include "area.h"

namespace hls {
// Allocate each operation with a resource type, and set limit to resource
// instances. Scheduling may only partially use allocation result.
// Returns 0 on success, -1 on error.
int allocate(const HLSInput &hls_input, HLSOutput &hls_output) {
    auto area_allocator = AreaAllocator(hls_input);
    area_allocator.allocate_type();
    area_allocator.allocate_inst();
    if (!area_allocator.validate())
        return -1;
    area_allocator.copyout(hls_output);
    return 0;
}

};  // namespace hls
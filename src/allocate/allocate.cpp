#include <iostream>

#include "algo.h"
#include "io.h"
#include "area.h"
#include "perf.h"

namespace hls {
// Allocate each operation with a resource type, and set limit to resource
// instances. Scheduling may only partially use allocation result.
// Returns 0 on success, -1 on error.
int allocate(const HLSInput &hls_input, HLSOutput &hls_output) {
    // auto allocator = AreaAllocator(hls_input);
    // allocator.allocate_type();
    // allocator.allocate_inst();
    auto allocator = PerfAllocator(hls_input);
    allocator.allocate_type(hls_input.area_limit);
    allocator.allocate_inst();
    if (!allocator.validate()) {
        std::cerr << "Errors: Allocate\n";
        return -1;
    }
    allocator.copyout(hls_output);
    return 0;
}

};  // namespace hls
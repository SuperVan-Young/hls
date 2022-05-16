#include "area.h"

static const int MAX_AREA = 1 << 20;

namespace hls {

// Allocate operation with resource of minimum area
void AreaAllocator::allocate_type() {
    vector<int> areas(n_op_type, MAX_AREA);

    // scan through each resource type and allocate to operation type
    for (auto &rt : hin->resource_types) {
        for (auto &comp_op : rt.comp_ops) {
            int area = rt.area;
            if (area < areas[comp_op]) {
                areas[comp_op] = area;
                ot2rtid[comp_op] = rt.rtid;
            }
        }
    }
}

// Allocate each type of operation with one instance, if it needs any
void AreaAllocator::allocate_inst() {
    for (int i = 0; i < n_op_type; i++) {
        int rt = ot2rtid[i];
        if (rt != -1) insts[i]++;
    }
}

// Validate the allocation
// Check if all operations are allocated and if area limit is violated
bool AreaAllocator::validate() {
    int total = 0;

    for (int op = 0; op < n_op_type; op++) {
        OpCategory op_cate = hin->op_types[op];
        int rt = ot2rtid[op];

        // check if operation is allocated
        if (rt == -1) {
            if (!(op_cate == OP_BRANCH || op_cate == OP_ALLOCA ||
                  op_cate == OP_PHI)) {
                return false;
            }
        }

        // add area to total
        const ResourceType &rtype = hin->resource_types[rt];
        total += rtype.area * insts[op];
    }

    if (total > hin->area_limit) return false;
    return true;
}

// Write result to hls output
// By default, we allocate each optype some exclusive instances,
// then gather those of the same type.
// We take this kind of reusing as an extra opportunity for scheduling.
void AreaAllocator::copyout(HLSOutput &hout) {
    for (int op = 0; op < n_op_type; op++) {
        hout.ot2rtid[op] = ot2rtid[op];
        hout.insts[op] = insts[op];

        int rt = ot2rtid[op];
        if (rt != -1)
            hout.rinsts[rt] += insts[op];
    }
}

// Display allocated result
void AreaAllocator::print(bool verbose) const {
    using std::cerr;
    using std::endl;
    cerr << "Allocation Result" << endl;
    cerr << "Optype | Resource Type | num of insts" << endl;
    for (int i = 0; i < n_op_type; i++) {
        cerr << i << ": " << ot2rtid[i] << ", " << insts[i] << endl;
        if (verbose) {
            int rtid = ot2rtid[i];
            if (rtid != - 1)
                hin->resource_types[rtid].print();
            cerr << endl;
        }
    }
}

}  // namespace hls

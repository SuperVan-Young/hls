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
                opid2rtid[comp_op] = rt.rtid;
            }
        }
    }
}

// Allocate each type of operation with one instance, if it needs any
void AreaAllocator::allocate_inst() {
    for (int i = 0; i < n_op_type; i++) {
        int rt = opid2rtid[i];
        if (rt != -1) insts[i]++;
    }
}

// Validate the allocation
// Check if all operations are allocated and if area limit is violated
bool AreaAllocator::validate() {
    int total = 0;

    for (int op = 0; op < n_op_type; op++) {
        OpCategory op_cate = hin->op_types[op];
        int rt = opid2rtid[op];

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
void AreaAllocator::copyout(HLSOutput &hout) {
    hout.op2rs.resize(n_op_type, 0);
    hout.insts.resize(hin->n_resource_type, vector<ResourceInst>());

    for (int op = 0; op < n_op_type; op++) {
        int rt = opid2rtid[op];
        hout.op2rs[op] = rt;
        if (rt != -1) {
            ResourceInst inst(rt, op);
            hout.insts[rt].push_back(inst);
        }
    }
}

// Display allocated result
void AreaAllocator::print(bool verbose) const {
    using std::cout;
    using std::endl;
    cout << "Allocation Result" << endl;
    for (int i = 0; i < n_op_type; i++) {
        cout << i << ": " << opid2rtid[i] << ", " << insts[i] << endl;
        if (verbose) {
            int rtid = opid2rtid[i];
            if (rtid != - 1)
                hin->resource_types[rtid].print();
            cout << endl;
        }
    }
}

}  // namespace hls

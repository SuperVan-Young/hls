#include "list.h"

#include <queue>
#include <vector>
#include <iostream>

namespace hls {

typedef float (*PtrSchedPriorFunc)(const Operation &, const HLSInput &,
                                   const HLSOutput &);

inline const ResourceType &op2rs(const Operation &op, const HLSInput &hin,
                                 const HLSOutput &hout) {
    int rtid = hout.op2rs[op.opid];
    return hin.resource_types[rtid];
}

float spf_latency(const Operation &op, const HLSInput &hin,
                  const HLSOutput &hout) {
    const ResourceType &rs = op2rs(op, hin, hout);
    return -rs.latency;  // smaller latency -> bigger priority
}

// List Scheduling.
// Returns 0 on success, -1 on errors.
int schedule_list(HLSInput &hin, HLSOutput &hout,
                  std::vector<OperationSched> &scheds,
                  SchedPriorFuncType functype) {
    // topology sort the cdfg first
    std::vector<TopoNode>cdfg = gather_CDFG(hin);
    std::vector<int> toposort(hin.n_operation);
    if (topology_sort(cdfg, toposort) == -1) {
        std::cerr << "Error: schedule_list" << std::endl;
        return -1;
    }

    

    return 0;
}

}  // namespace hls

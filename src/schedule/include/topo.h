#ifndef HLS_SCHEDULE_TOPO_H
#define HLS_SCHEDULE_TOPO_H

#include "io.h"

namespace hls {

// A general input type for topology sort
class TopoNode {
   public:
    int bbid;
    int id;
    int in;
    std::vector<int> out;

    // Default order: bb id first, op id second
    bool operator<(const TopoNode &x) const {
        if (bbid != x.bbid)
            return bbid < x.bbid;
        else
            return id < x.id;
    }
    bool operator>(const TopoNode &x) const {
        if (bbid != x.bbid)
            return bbid > x.bbid;
        else
            return id > x.id;
    }
};

std::vector<TopoNode> gather_CDFG(const HLSInput &);

int topology_sort(std::vector<TopoNode> &, std::vector<int> &);

std::vector<int> order_bb(const HLSInput &, const std::vector<int> &);

std::vector<int> subgraph_bb(const std::vector<int> &,
                             const std::vector<int> &);

};  // namespace hls

#endif
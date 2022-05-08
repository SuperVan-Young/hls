#ifndef HLS_SCHEDULE_TOPO_H
#define HLS_SCHEDULE_TOPO_H

#include "io.h"

namespace hls {

// A general input type for topology sort
class TopoNode {
   public:
    int id;
    int in;
    std::vector<int> out;

    // Default order is comparing by id
    bool operator<(const TopoNode &x) const { return id < x.id; }
    bool operator>(const TopoNode &x) const { return id > x.id; }
};

std::vector<TopoNode> gather_CDFG(const std::vector<Operation> &);

int topology_sort(std::vector<TopoNode> &, std::vector<int> &);

}; // namespace hls

#endif
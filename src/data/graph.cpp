#include "io.h"

namespace hls {
AdjacentList HLSInput::build_induced_graph(int bbid) {
    AdjacentList list;
    const BasicBlock &bb = blocks[bbid];

    // add nodes
    for (auto v : bb.ops)
        list.insert(std::make_pair(v, AdjacentNode(0, vector<int>())));

    // add edges
    for (auto v : bb.ops) {
        const auto &op = operations[v];
        for (auto u : op.inputs) {
            if (list.count(u)) {  // prev vertex, ignore -1 automatically
                list[v].first++;              // in degree ++
                list[u].second.push_back(v);  // append to outs
            }
        }
    }
    
    return list;
}

}  // namespace hls

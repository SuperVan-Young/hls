#ifndef HLS_GRAPH_H
#define HLS_GRAPH_H

#include <map>
#include <vector>
#include <queue>

#include "io.h"

using std::map;
using std::pair;
using std::vector;
using std::queue;

namespace hls {
typedef pair<int, vector<int>> AdjacentNode;
typedef map<int, AdjacentNode> AdjacentList;

AdjacentList build_induced_graph(int bbid, const HLSInput &hin);

int topology_sort(AdjacentList g, vector<int> &out);

int topology_sort(AdjacentList g, const HLSInput &hin,
                  vector<vector<int>> &out);

};  // namespace hls

#endif
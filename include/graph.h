#ifndef HLS_GRAPH_H
#define HLS_GRAPH_H

#include <map>
#include <queue>
#include <vector>
#include <algorithm>

#include "io.h"

using std::map;
using std::pair;
using std::queue;
using std::vector;

namespace hls {
typedef pair<int, vector<int>> AdjacentNode;
typedef map<int, AdjacentNode> AdjacentList;

AdjacentList build_induced_graph(int bbid, const HLSInput &hin);

int topology_sort(AdjacentList g, vector<int> &out);

int topology_sort(AdjacentList g, const HLSInput &hin,
                  vector<vector<int>> &out);

int topology_sort(AdjacentList g, const HLSInput &hin,
                  const vector<int> &ot2rtid, vector<vector<int>> &out);

vector<pair<int, int>> sort_interval_graph(const HLSOutput &hout);

};  // namespace hls

#endif
#ifndef HLS_BIND_H
#define HLS_BIND_H

#include <vector>
#include <algorithm>

#include "io.h"
#include "algo.h"

using std::vector;

using std::pair;

namespace hls {

class ConflictGraph {
    public:
     int n_vertex = 0;
     int max_color = 0;
     vector<vector<int>> edges;
     vector<int> colors;

    void setup(int n_operation) {
        n_vertex = n_operation;
        max_color = 0;
        edges.resize(n_vertex, vector<int>());
        colors.resize(n_vertex, -1);
    }

    bool need_binding(int opid, const HLSInput &hin);
    
    bool check_conflict(int opid1, int opid2, const HLSInput &hin, const HLSOutput &hout);

    void add_conflict(int opid1, int opid2);

    int add_color(int op);
};

vector<pair<int, int>> sort_interval_graph(const HLSOutput &hout);


}; // namespace hls

#endif
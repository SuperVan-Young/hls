#ifndef HLS_BIND_H
#define HLS_BIND_H

#include <vector>
#include <algorithm>

#include "io.h"
#include "algo.h"

using std::vector;
using std::pair;

namespace hls {

// Operation Conflict graph
class ConflictGraph {
    public:
     int n_vertex = 0;
     int max_color = 0;
     vector<vector<int>> edges;
     vector<int> colors;

    ConflictGraph(int n_operation) {
        n_vertex = n_operation;
        max_color = 0;
        edges.resize(n_vertex, vector<int>());
        colors.resize(n_vertex, -1);
    }

    bool check_conflict(int opid1, int opid2, const HLSInput &hin, const HLSOutput &hout);

    void add_conflict(int opid1, int opid2);

    int add_color(int op);
};

// Binding operations to resource instances
// Base Binder thinks each optype has exclusive resource instances
class BaseBinder {
    public:
    const HLSInput *hin;
    const HLSOutput *hout;
    int n_operation;
    int n_op_type;
    vector<int> binds;


    BaseBinder(const HLSInput &hin, const HLSOutput &hout) {
        this->hin = &hin;
        this->hout = &hout;
        this->n_operation = hin.n_operation;
        this->n_op_type = hin.n_op_type;
        binds.resize(n_operation, -1);
    }

    int bind();

    void copyout(HLSOutput &hout);
};

vector<pair<int, int>> sort_interval_graph(const HLSOutput &hout);


}; // namespace hls

#endif
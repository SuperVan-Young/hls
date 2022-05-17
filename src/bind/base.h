#ifndef HLS_BIND_H
#define HLS_BIND_H

#include <algorithm>
#include <vector>

#include "algo.h"
#include "io.h"

using std::pair;
using std::vector;

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

    void add_conflict(int opid1, int opid2) {
        edges[opid1].push_back(opid2);
        edges[opid2].push_back(opid1);
    }

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

    bool check_conflict(int opid1, int opid2);

    int bind();

    void copyout(HLSOutput &hout);
};

// Binding operations to resource instances
// Allows sharing resources between compatible operations
class RBinder : public BaseBinder {
   public:
    RBinder(const HLSInput &hin, const HLSOutput &hout)
        : BaseBinder(hin, hout) {}

    bool check_conflict(int opid1, int opid2);

    int bind();
};

vector<pair<int, int>> sort_interval_graph(const HLSOutput &hout);

};  // namespace hls

#endif
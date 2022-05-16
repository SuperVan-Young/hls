#ifndef HLS_ALLOCATE_PERF_H
#define HLS_ALLOCATE_PERF_H

#include <vector>
#include <queue>
#include <map>
#include <cmath>

#include "area.h"
#include "io.h"

using std::vector;
using std::queue;
using std::map;

namespace hls {

// Abstracted CDFG of a basic block
class AbstractedCDFG {
public:
    int n_op_type = 0;
    int bbid = 0;
    const HLSInput *hin = nullptr;
    vector<vector<int>> dependencies; // dependence tree
    AbstractedCDFG(int n_op_type, int bbid, const HLSInput &hin) {
        this->n_op_type = n_op_type;
        this->bbid = bbid;
        this->hin = &hin;
        dependencies.resize(n_op_type, vector<int>());
        setup();
    }

    void setup();
    void print();
    float estimate_perf(int optype, const ResourceType &rtype, int num);

};

// Allocate each operation w.r.t. its expected latency and area.
class PerfAllocator : public AreaAllocator {
   private:
    int n_block = 0;
    vector<AbstractedCDFG> cdfgs;

   public:
    PerfAllocator(const HLSInput &hin) : AreaAllocator(hin) {
        // initialize abstracted CDFG
        n_block = hin.n_block;
        for (int i = 0; i < n_block; i++) {
            cdfgs.push_back(AbstractedCDFG(n_op_type, i, hin));
        }
    }
    void allocate_type(int area_limit);
    void allocate_inst();
    float estimate_perf(int optype, const ResourceType &rtype, int num);
};

// Evaluation on adding one more resource instance
class IncrNode {
   public:
    int optype;
    int area;
    int num;
    float oldperf;
    float newperf;

    // maximize performance gain, minimize area cost
    bool operator<(const IncrNode &x) const;
};

}  // namespace hls

#endif
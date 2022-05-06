// io.hpp
// Basic data structures as well as I/O.
#ifndef HLS_IO_HPP
#define HLS_IO_HPP

#include <fstream>
#include <vector>

namespace hls {

enum OpType {
    OP_BRANCH = 1,  // cond, true-branch, false-branch -> None
    OP_ALLOCA,      // k inputs -> None
    OP_LOAD,        // k inputs as indices -> array element
    OP_STORE,       // k inputs as indices -> None
    OP_PHI,         // k inputs -> the chosen input
                    // NO SCHEDULING OR MAPPING on PHI!
    OP_ARITHM,      // k inputs -> out
    OP_BOOL,        // k inputs (from bool or compare) -> out
    OP_COMPARE,     // in1, in2 -> out
};

// Description of a type of resources
class Resource {
   public:
    bool is_sequential;
    int area;
    float delay;                // need delay for an available result
    int latency;                // 0 if not sequential
    bool is_pipelined;          // false if not sequential; II = 1
    int n_comp_op;              // number of compatible operations
    std::vector<int> comp_ops;  // compatible operations

    Resource(std::ifstream &);
};

// Description of a basic block
class BasicBlock {
   public:
    int n_op_in_block;
    int n_pred;       // number of predessor basic blocks, n_pred >= 0
    int n_succ;       // number of successor basic blocks, 0<= n_succ <= 2
                      // a block with 2 succ must have a branch op in it
    float exp_times;  // the expected times the basic block will be executed
                      // This relates to performance evaluation
    std::vector<int> ops;    // id of ops
    std::vector<int> preds;  // id of bbs
    std::vector<int> succs;  // id of bbs

    BasicBlock(std::ifstream &);
};

// Description of an operation
class Operation {
   public:
    OpType optype;
    int n_inputs;
    std::vector<int> inputs;  // id of ops

    Operation(std::ifstream &);
};

// Input of resource libraries and CDFG
class HLSInput {
   public:
    int n_resource_type;  // length of resources array
    int n_op_type;    // number of operations, each type from the 8 categories
    float target_cp;  // maximum ns in one cycle
    int area_limit;   // total area limit
    int n_block;      // length of blocks
    int n_operation;  // total operations
    std::vector<OpType> op_types;  // category of each operation type, 1~8
    std::vector<Resource> resources;
    std::vector<BasicBlock> blocks;
    std::vector<Operation> operations;

    HLSInput(char *);
};
class HLSOutput {
   public:
    int n_operation;
    int n_resource_type;
    std::vector<int> sched_cycles;    // starting cycle of op_i
    std::vector<int> sched_types;     // the resource type op_i binds to
    std::vector<int> sched_insts;     // the resource instance op_i binds to
    std::vector<int> resource_insts;  // number of instances for rtype_i

    void output();
};
};  // namespace hls

#endif
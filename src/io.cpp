#include "io.hpp"

#include <fstream>
#include <iostream>

hls::HLSInput::HLSInput(char *filename) {
    std::ifstream fin(filename);

    // Resource library description
    fin >> n_resource_type >> n_op_type >> target_cp >> area_limit;
    for (int i = 0; i < n_resource_type; i++) {
        resources.push_back(hls::Resource(fin));
    }

    // CDFG description
    fin >> n_block >> n_operation;
    for (int i = 0; i < n_block; i++) {
        blocks.push_back(hls::BasicBlock(fin));
    }
    for (int i = 0; i < n_operation; i++) {
        operations.push_back(hls::Operation(fin));
    }

    fin.close();
}

hls::Resource::Resource(std::ifstream &fin) {
    int seq, pipe;
    fin >> seq >> area;
    is_sequential = (seq != 0);

    if (is_sequential) {
        fin >> latency >> delay >> pipe;
        is_pipelined = (pipe != 0);
    } else {
        fin >> delay;
    }

    fin >> n_comp_op;
    for (int i = 0; i < n_comp_op; i++) {
        int x;
        fin >> x;
        comp_ops.push_back(x);
    }
}

hls::BasicBlock::BasicBlock(std::ifstream &fin) {
    fin >> n_op_in_block >> n_pred >> n_succ >> exp_times;
    for (int i = 0; i < n_op_in_block; i++) {
        int op;
        fin >> op;
        ops.push_back(op);
    }
    for (int i = 0; i < n_pred; i++) {
        int pred;
        fin >> pred;
        preds.push_back(pred);
    }
    for (int i = 0; i < n_succ; i++) {
        int succ;
        fin >> succ;
        succs.push_back(succ);
    }
}

hls::Operation::Operation(std::ifstream &fin) {
    int type;
    fin >> type;
    optype = (hls::OpType)type;
    fin >> n_inputs;
    for (int i = 0; i < n_inputs; i++) {
        int input;
        fin >> input;
        inputs.push_back(input);
    }
}

void hls::HLSOutput::output() {
    // scheduling result
    for (int i = 0; i < n_operation; i++) std::cout << sched_cycles[i] << ' ';
    std::cout << std::endl;

    // allocation result
    for (int i = 0; i < n_resource_type; i++)
        std::cout << resource_insts[i] << ' ';
    std::cout << std::endl;

    // binding result
    // for operations of 1-5 categories, sched_types == -1
    // its sched_inst is meaningless
    for (int i = 0; i < n_operation; i++) {
        int sched_type = sched_types[i];
        if (sched_type != -1) {
            std::cout << sched_type << ' ' << sched_insts[i] << std::endl;
        } else {
            std::cout << sched_type << std::endl;
        }
    }
}
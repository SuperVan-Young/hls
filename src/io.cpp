#include "io.h"

#include <cstring>
#include <fstream>
#include <iostream>

void input_array(std::ifstream &fin, std::vector<int> &array, int len) {
    for (int i = 0; i < len; i++) {
        int tmp;
        fin >> tmp;
        array.push_back(tmp);
    }
}

void output_array(std::string name, std::vector<int> &array, int len) {
    std::cout << name << ": ";
    for (int i = 0; i < len; i++) {
        std::cout << array[i] << ' ';
    }
    std::cout << std::endl;
}

hls::HLSInput::HLSInput(char *filename) {
    std::ifstream fin(filename);

    // Resource library description
    fin >> n_resource_type >> n_op_type >> target_cp >> area_limit;
    for (int i = 0; i < n_resource_type; i++) {
        resources.push_back(hls::Resource(fin));
    }

    // CDFG description
    fin >> n_block >> n_operation;
    // Oeration types
    for (int i = 0; i < n_op_type; i++) {
        int type;
        fin >> type;
        op_types.push_back((OpType)type);
    }
    // Blocks
    for (int i = 0; i < n_block; i++) {
        blocks.push_back(hls::BasicBlock(fin));
    }
    // Operations
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
    input_array(fin, comp_ops, n_comp_op);
}

hls::BasicBlock::BasicBlock(std::ifstream &fin) {
    fin >> n_op_in_block >> n_pred >> n_succ >> exp_times;
    input_array(fin, ops, n_op_in_block);
    input_array(fin, preds, n_pred);
    input_array(fin, succs, n_succ);
}

hls::Operation::Operation(std::ifstream &fin) {
    fin >> optype >> n_inputs;
    input_array(fin, inputs, n_inputs);
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
    // for operations of categories 1-5 and 8, sched_types == -1
    // its sched_inst is meaningless
    // only arithmetic and boolean operations need to bind a resource instance
    for (int i = 0; i < n_operation; i++) {
        int sched_type = sched_types[i];
        if (sched_type != -1) {
            std::cout << sched_type << ' ' << sched_insts[i] << std::endl;
        } else {
            std::cout << sched_type << std::endl;
        }
    }
}

void hls::Resource::print() {
    std::cout << "is_sequential: " << (int)is_sequential << std::endl;
    std::cout << "area: " << area << std::endl;

    if (is_sequential) std::cout << "latency: " << latency << std::endl;
    std::cout << "delay: " << delay << std::endl;
    if (is_sequential)
        std::cout << "is_pipelined: " << (int)is_pipelined << std::endl;

    output_array("Compatible Operation", comp_ops, n_comp_op);
}

void hls::BasicBlock::print() {
    std::cout << "n_op_in_block: " << n_op_in_block << std::endl;
    std::cout << "n_pred: " << n_pred << std::endl;
    std::cout << "n_succ: " << n_succ << std::endl;
    std::cout << "exp_times: " << exp_times << std::endl;

    output_array("Operations", ops, n_op_in_block);
    output_array("Preds", preds, n_pred);
    output_array("Succs", succs, n_succ);
}

void hls::Operation::print() {
    std::cout << "optype: " << optype << std::endl;
    std::cout << "n_inputs: " << n_inputs << std::endl;

    output_array("Inputs", inputs, n_inputs);
}

void hls::HLSInput::print() {
    std::cout << "HLS INPUTS" << std::endl;

    std::cout << "n_resource_type: " << n_resource_type << std::endl;
    std::cout << "n_op_type: " << n_op_type << std::endl;
    std::cout << "target_cp: " << target_cp << std::endl;
    std::cout << "area_limit: " << area_limit << std::endl;
    std::cout << "n_block: " << n_block << std::endl;
    std::cout << "n_operation: " << n_operation << std::endl;

    std::cout << std::endl;

    for (int i = 0; i < n_op_type; i++) {
        std::cout << "Operation " << i << ": ";
        std::string s = "Error";
        switch (op_types[i]) {
            case OP_BRANCH:
                s = "branch     (1)";
                break;
            case OP_ALLOCA:
                s = "allocation (2)";
                break;
            case OP_LOAD:
                s = "load       (3)";
                break;
            case OP_STORE:
                s = "store      (4)";
                break;
            case OP_PHI:
                s = "phi        (5)";
                break;
            case OP_ARITHM:
                s = "arithmetic (6)";
                break;
            case OP_BOOL:
                s = "boolean    (7)";
                break;
            case OP_COMPARE:
                s = "compare    (8)";
                break;
        }
        std::cout << s << std::endl;
    }
    std::cout << std::endl;

    for (int i = 0; i < n_resource_type; i++) {
        std::cout << "Resource " << i << std::endl;
        resources[i].print();
        std::cout << std::endl;
    }
    for (int i = 0; i < n_block; i++) {
        std::cout << "Block " << i << std::endl;
        blocks[i].print();
        std::cout << std::endl;
    }
    for (int i = 0; i < n_operation; i++) {
        std::cout << "Operation " << i << std::endl;
        operations[i].print();
        std::cout << std::endl;
    }
}
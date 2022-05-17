#include <iostream>

#include "allocate/ilp.h"
#include "bind/base.h"
#include "io.h"
#include "schedule/sdc.h"

int main(int argc, char* argv[]) {
    if (argc != 2) exit(-1);
    hls::HLSInput hls_input(argv[1]);
    // hls_input.print();

    hls::HLSOutput hls_output(hls_input);

    // allocate rtype, without setting num of instances
    hls::ILPAllocator allocator(hls_input);
    if (allocator.allocate_resource_type() < 0) {
        cerr << "Main Error: Allocating Resource types!" << endl;
        exit(-1);
    }
    if (allocator.allocate_operation_type() < 0) {
        cerr << "Main Error: Allocating Operation types!" << endl;
        exit(-1);
    }
    allocator.copyout(hls_output);

    // Scheduling and binding, regardless of area limit
    hls::SDCScheduler scheduler(hls_input, hls_output, false);
    hls::RBinder binder(hls_input, hls_output);

    if (scheduler.schedule() < 0) {
        cerr << "Main Error: Scheduling." << endl;
        exit(-1);
    }
    scheduler.copyout(hls_output);
    if (binder.bind() < 0) {
        cerr << "Main Error: Binding." << endl;
        exit(-1);
    }
    binder.copyout(hls_output);

    // check area constraints
    int res = allocator.allocate_insts_bound(hls_output.rinsts);
    if (res < 0) {
        cerr << "Main Error: Allocate insts bound" << endl;
        exit(-1);
    } else if (res == 0) {
        scheduler.rlimit = true;
        if (scheduler.schedule() < 0) {
            cerr << "Main Error: Scheduling." << endl;
            exit(-1);
        }
        scheduler.copyout(hls_output);
        if (binder.bind() < 0) {
            cerr << "Main Error: Binding." << endl;
            exit(-1);
        }
        binder.copyout(hls_output);
    }

    hls_output.output();
    return 0;
}
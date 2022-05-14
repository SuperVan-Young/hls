#include <iostream>

#include "io.h"
#include "algo.h"

// #define DEBUG_MAIN
using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {
    if (argc != 2)
        exit(-1);
    hls::HLSInput hls_input(argv[1]);
    // hls_input.print();

    hls::HLSOutput hls_output(hls_input);

    hls::allocate(hls_input, hls_output);
#ifdef DEBUG_HLS_ALLOCATION
    cerr << "After Allocation" << endl;
    hls_output.output();
    cerr << endl;
#endif

    hls::schedule(hls_input, hls_output);
#ifdef DEBUG_HLS_SCHEDULE
    cerr << "After Scheduling" << endl;
    hls_output.output();
    cerr << endl;
#endif

    hls::bind(hls_input, hls_output);
#ifdef DEBUG_HLS_BIND
    cerr << "After Binding" << endl;
    hls_output.output();
    cerr << endl;
#endif

    hls_output.output();
    return 0;
}
#include <iostream>

#include "io.h"
#include "algo.h"

// #define DEBUG_MAIN
using std::cout;
using std::endl;

int main(int argc, char* argv[]) {
    if (argc != 2)
        exit(-1);
    hls::HLSInput hls_input(argv[1]);
// #ifdef DEBUG_MAIN
//     hls_input.print();
//     cout << endl;
// #endif

    hls::HLSOutput hls_output(hls_input);

    hls::allocate(hls_input, hls_output);
#ifdef DEBUG_MAIN
    cout << "After Allocation" << endl;
    hls_output.output();
    cout << endl;
#endif

    hls::schedule(hls_input, hls_output);
#ifdef DEBUG_MAIN
    cout << "After Scheduling" << endl;
    hls_output.output();
    cout << endl;
#endif

    hls::bind(hls_input, hls_output);
#ifdef DEBUG_MAIN
    cout << "After Binding" << endl;
    hls_output.output();
    cout << endl;
#endif

    hls_output.output();
    return 0;
}
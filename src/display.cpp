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
    hls_input.print();
    return 0;
}
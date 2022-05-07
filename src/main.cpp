#include <iostream>

#include "io.h"
#include "algo.h"

int main(int argc, char* argv[]) {
    if (argc != 2)
        exit(-1);
    hls::HLSInput hls_input(argv[1]);
    hls_input.print();

    hls::HLSOutput hls_output;
    hls_output.setup(hls_input);
    hls::allocate(hls_input, hls_output);
    hls_output.output();

    return 0;
}
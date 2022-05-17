#include <iostream>

#include "io.h"

using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {
    if (argc != 2)
        exit(-1);
    hls::HLSInput hls_input(argv[1]);
    hls_input.print();
    return 0;
}
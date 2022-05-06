#include <iostream>

#include "io.h"
int main(int argc, char* argv[]) {
    if (argc != 2)
        exit(-1);
    hls::HLSInput hls_input(argv[1]);
    return 0;
}
#include "algo.h"
#include "base.h"
#include "io.h"

namespace hls {

int bind(const HLSInput &hls_input, HLSOutput &hls_output) {
    auto binder = BaseBinder(hls_input, hls_output);
    if (binder.bind() < 0) {
        std::cerr << "Error: bind" << std::endl;
        return -1;
    }
    binder.copyout(hls_output);
    return 0;
}

}  // namespace hls

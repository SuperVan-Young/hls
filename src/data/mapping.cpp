#include "io.h"

namespace hls
{
OpCategory HLSInput::get_opcate(int opid) const {
    return op_types[operations[opid].optype];
}

bool HLSInput::need_schedule(OpCategory opcate) const {
    return !(opcate == OP_ALLOCA || opcate == OP_BRANCH || opcate == OP_PHI);
}

bool HLSInput::need_bind(OpCategory opcate) const {
    return (opcate == OP_ARITHM || opcate == OP_BOOL || opcate == OP_COMPARE);
}


} // namespace hls

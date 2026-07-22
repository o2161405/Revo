#pragma once

#include "abstract/FunctionImpl.hh"
#include "instruction/Instruction.hh"

#include <vector>

namespace Revo {

struct DecoderResult {
    std::vector<FunctionImpl<DecodedInstruction>> functions;
};

} // namespace Revo

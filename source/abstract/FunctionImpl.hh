#pragma once

#include "elf/Types.hh"

#include <flat_map>
#include <vector>

namespace Revo {

template <typename TInstruction>
struct FunctionImpl {
    std::vector<TInstruction> instructions;
    std::flat_map<u32 /*relative offset*/, std::vector<ELF::Rela>> relocations;
    u32 offset;
    u32 size;
};

} // namespace Revo

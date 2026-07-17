#pragma once

#include <flat_map>
#include <vector>

namespace Revo {

template <typename TInstruction, typename TOffset, typename TRela>
struct FunctionImpl {
    std::vector<TInstruction> instructions;
    std::flat_map<TOffset, std::vector<TRela>> relocations;
    u32 offset;
    u32 size;
};

} // namespace Revo

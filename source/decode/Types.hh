#pragma once

namespace Revo::Decode {

using RelativeOffset = u32;

struct Instruction {
    static constexpr auto MAX_OPERANDS{5uz};

    PPC::Mnemonic mnemonic;
    std::inplace_vector<PPC::Operand, MAX_OPERANDS> operands{};
    PPC::Operand::Behavior behaviors{};
};

struct Function {
    std::vector<Instruction> instructions;
    std::flat_map<RelativeOffset, std::vector<ELF::Rela>> relocations;
    u32 offset;
    u32 size;
};

} // namespace Revo::Decode

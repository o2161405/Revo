#pragma once

#include "decode/Types.hh"

namespace Revo::CFG {

enum class Terminator : u8 {
    Fallthrough,
    Call,
    Branch,
    Return,
    Indirect,
};

[[nodiscard]] constexpr Terminator
terminator(const Decode::Instruction& instruction) {
    if (instruction.is_call()) {
        return Terminator::Call;
    }
    if (instruction.branch_destination()) {
        return Terminator::Branch;
    }
    if (!instruction.indirect_branch_source) {
        return Terminator::Fallthrough;
    }
    if (*instruction.indirect_branch_source == PPC::Register::SPR::LR) {
        return Terminator::Return;
    }

    return Terminator::Indirect;
}

[[nodiscard]] constexpr bool
falls_through(const Decode::Instruction& instruction) {
    const auto instruction_terminator = terminator(instruction);
    return instruction_terminator == Terminator::Fallthrough //
        || instruction_terminator == Terminator::Call || instruction.conditional_branch();
}

} // namespace Revo::CFG

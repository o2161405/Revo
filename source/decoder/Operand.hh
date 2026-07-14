#pragma once

#include <variant>

#include "decoder/Register.hh"

namespace Revo::Decoder {

struct Operand {
    // todo: can the types in these structs be derived from Instruction::INSTRUCTION_WIDTH
    // whilst being clean? i've tried already but couldn't do it cleanly
    struct Immediate {
        s32 value;
    };

    struct BranchDestination {
        u32 address;
    };

    enum class Type {
        None,
        GPR,
        FPR,
        CR,
        Immediate,
        BranchDestination,
    };

    /* clang-format off */
    using Variant = std::variant<
        std::monostate,
        Register::GPR,
        Register::FPR,
        Register::CR,
        Immediate,
        BranchDestination
    >;
    /* clang-format on */

    Variant value;
};

} // namespace Revo::Decoder

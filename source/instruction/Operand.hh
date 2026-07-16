#pragma once

#include <variant>

#include "instruction/Register.hh"

namespace Revo {

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

    enum class Behavior : u8 {
        None,
        Record,
        Link,
        Absolute,
        Overflow,
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

    template <Type TType>
    [[nodiscard]] static constexpr Operand
    get(auto value) {
        if constexpr (TType == Type::GPR) {
            return Operand{.value = Register::GPR{value}};
        }
        else if constexpr (TType == Type::FPR) {
            return Operand{.value = Register::FPR{value}};
        }
        else if constexpr (TType == Type::CR) {
            return Operand{.value = Register::CR{value}};
        }
        else if constexpr (TType == Type::Immediate) {
            return Operand{.value = Immediate{value}};
        }
        else if constexpr (TType == Type::BranchDestination) {
            return Operand{.value = BranchDestination{static_cast<u32>(value)}};
        }
        else {
            static_assert(false, "Type has no operand equivalent");
        }
    }

    Variant value;
};

} // namespace Revo

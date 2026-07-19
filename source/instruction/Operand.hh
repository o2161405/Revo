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
        SplitImmediate,
    };

    enum class Role : u8 {
        None,
        Read,
        Write,
    };

    /* clang-format off */
    enum class Behavior : u8 {
        None     = 0,
        Record   = 1 << 0,
        Link     = 1 << 1,
        Absolute = 1 << 2,
        Overflow = 1 << 3,
    };

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
    get(auto value, Role role = Role::None) {
        if constexpr (TType == Type::GPR) {
            return Operand{.value = Register::GPR{value}, .role = role};
        }
        else if constexpr (TType == Type::FPR) {
            return Operand{.value = Register::FPR{value}, .role = role};
        }
        else if constexpr (TType == Type::CR) {
            return Operand{.value = Register::CR{value}, .role = role};
        }
        else if constexpr (TType == Type::Immediate) {
            return Operand{.value = Immediate{value}, .role = role};
        }
        else if constexpr (TType == Type::BranchDestination) {
            return Operand{.value = BranchDestination{static_cast<u32>(value) << 2}, .role = role};
        }
        else if constexpr (TType == Type::SplitImmediate) {
            constexpr auto SHIFT{5uz};
            constexpr u32 HALF_MASK = (1 << SHIFT) - 1;

            const auto raw = static_cast<u32>(value);
            return Operand{
                .value = Immediate{static_cast<s32>(((raw & HALF_MASK) << SHIFT) | (raw >> SHIFT))},
                .role = role};
        }
        else {
            static_assert(false, "Type has no operand equivalent");
        }
    }

    Variant value;
    Role role{Role::None};
};

[[nodiscard]] constexpr Operand::Behavior
operator|(Operand::Behavior rhs, Operand::Behavior lhs) {
    return static_cast<Operand::Behavior>(std::to_underlying(rhs) | std::to_underlying(lhs));
}

[[nodiscard]] constexpr Operand::Behavior
operator&(Operand::Behavior rhs, Operand::Behavior lhs) {
    return static_cast<Operand::Behavior>(std::to_underlying(rhs) & std::to_underlying(lhs));
}

constexpr Operand::Behavior&
operator|=(Operand::Behavior& rhs, Operand::Behavior lhs) {
    return rhs = (rhs | lhs);
}

} // namespace Revo

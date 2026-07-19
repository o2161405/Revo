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
        SPR,
        CR,
        Immediate,
        BranchDestination,
        SplitImmediate,
    };

    /* clang-format off */
    enum class Access : u8 {
        None  = 0,
        Read  = 1 << 0,
        Write = 1 << 1,
    };

    [[nodiscard]] friend constexpr Access operator|(Access lhs, Access rhs) {
        return static_cast<Access>(std::to_underlying(lhs) | std::to_underlying(rhs));
    }

    [[nodiscard]] friend constexpr Access operator&(Access lhs, Access rhs) {
        return static_cast<Access>(std::to_underlying(lhs) & std::to_underlying(rhs));
    }

    friend constexpr Access& operator|=(Access& lhs, Access rhs) {
        return lhs = (lhs | rhs);
    }

    enum class Behavior : u8 {
        None     = 0,
        Record   = 1 << 0,
        Link     = 1 << 1,
        Absolute = 1 << 2,
        Overflow = 1 << 3,
    };

    [[nodiscard]] friend constexpr Behavior operator|(Behavior lhs, Behavior rhs) {
        return static_cast<Behavior>(std::to_underlying(lhs) | std::to_underlying(rhs));
    }

    [[nodiscard]] friend constexpr Behavior operator&(Behavior lhs, Behavior rhs) {
        return static_cast<Behavior>(std::to_underlying(lhs) & std::to_underlying(rhs));
    }

    friend constexpr Behavior& operator|=(Behavior& lhs, Behavior rhs) {
        return lhs = (lhs | rhs);
    }

    using Variant = std::variant<
        std::monostate,
        Register::GPR,
        Register::FPR,
        Register::SPR,
        Register::CR,
        Immediate,
        BranchDestination
    >;
    /* clang-format on */

    template <Type TType>
    [[nodiscard]] static constexpr Operand
    get(auto value, Access access = Access::None) {
        constexpr auto SHIFT{5uz};
        constexpr u32 HALF_MASK = (1 << 5) - 1;

        if constexpr (TType == Type::GPR) {
            return Operand{.value = Register::GPR{value}, .access = access};
        }
        else if constexpr (TType == Type::FPR) {
            return Operand{.value = Register::FPR{value}, .access = access};
        }
        else if constexpr (TType == Type::SPR) {
            const auto raw = static_cast<u32>(value);
            return Operand{
                .value = Register::SPR{static_cast<u16>(((raw & HALF_MASK) << 5) | (raw >> 5))},
                .access = access};
        }
        else if constexpr (TType == Type::CR) {
            return Operand{.value = Register::CR{value}, .access = access};
        }
        else if constexpr (TType == Type::Immediate) {
            return Operand{.value = Immediate{value}, .access = access};
        }
        else if constexpr (TType == Type::BranchDestination) {
            return Operand{
                .value = BranchDestination{static_cast<u32>(value) << 2}, .access = access};
        }
        else if constexpr (TType == Type::SplitImmediate) {
            const auto raw = static_cast<u32>(value);
            return Operand{
                .value = Immediate{static_cast<s32>(((raw & HALF_MASK) << SHIFT) | (raw >> SHIFT))},
                .access = access};
        }
        else {
            static_assert(false, "Type has no operand equivalent");
        }
    }

    Variant value;
    Access access{Access::None};
};

} // namespace Revo

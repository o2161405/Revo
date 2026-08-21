#pragma once

#include "ppc/Register.hh"

#include <utility>
#include <variant>

namespace Revo::PPC {

struct Operand {
    /* clang-format off */
    struct Immediate { s32 value; };
    struct BranchDestination { u32 address; };
    struct BranchOptions { u8 value; };

    enum class Access : u8 { //
        None      = 0,
        Read      = 1 << 0,
        Write     = 1 << 1,
        ReadWrite = Read | Write
    };

    enum class Behavior : u8 { //
        None     = 0,
        Record   = 1 << 0,
        Link     = 1 << 1,
        Absolute = 1 << 2,
        Overflow = 1 << 3
    };
    /* clang-format on */

    using Variant = std::variant< //
        Register::GPR, Register::FPR, Register::SPR, Register::CR, //
        Immediate, BranchDestination, BranchOptions>;

    Variant value;
    Access access{Access::None};

    template <typename TType>
    [[nodiscard]] constexpr bool
    is() const {
        return std::holds_alternative<TType>(value);
    }

    template <typename TType>
    [[nodiscard]] constexpr bool
    is(const TType& value) const {
        const auto* ptr = std::get_if<TType>(&this->value);
        return ptr && *ptr == value;
    }

    [[nodiscard]] constexpr std::optional<s32>
    immediate() const {
        if (const auto* ptr = std::get_if<Immediate>(&value)) {
            return ptr->value;
        }

        return std::nullopt;
    }

    [[nodiscard]] constexpr std::optional<Register::GPR>
    gpr() const {
        if (const auto* ptr = std::get_if<Register::GPR>(&value)) {
            return *ptr;
        }

        return std::nullopt;
    }
};

[[nodiscard]] constexpr Operand::Access
operator|(Operand::Access lhs, Operand::Access rhs) {
    return static_cast<Operand::Access>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr Operand::Access
operator&(Operand::Access lhs, Operand::Access rhs) {
    return static_cast<Operand::Access>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

constexpr Operand::Access&
operator|=(Operand::Access& lhs, Operand::Access rhs) {
    return lhs = (lhs | rhs);
}

[[nodiscard]] constexpr Operand::Behavior
operator|(Operand::Behavior lhs, Operand::Behavior rhs) {
    return static_cast<Operand::Behavior>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr Operand::Behavior
operator&(Operand::Behavior lhs, Operand::Behavior rhs) {
    return static_cast<Operand::Behavior>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

constexpr Operand::Behavior&
operator|=(Operand::Behavior& lhs, Operand::Behavior rhs) {
    return lhs = (lhs | rhs);
}

} // namespace Revo::PPC

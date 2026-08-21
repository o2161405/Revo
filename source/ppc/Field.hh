#pragma once

#include "ppc/Common.hh"
#include "ppc/Operand.hh"

#include <concepts>
#include <meta>

namespace Revo::PPC {

constexpr u32
swap_order(u32 raw) {
    constexpr auto SHIFT{5uz};
    constexpr u32 HALF_MASK = (1 << 5) - 1;

    return ((raw & HALF_MASK) << SHIFT) | (raw >> SHIFT);
}

struct FieldAccess {
    std::meta::info field;
    Operand::Access access;
};

struct FieldConstant {
    std::meta::info field;
    u32 value;
};

template <u8 TStartIndex, u8 TEndIndex, typename TDataType>
struct Field {
    using data_type = TDataType;
    static constexpr u8 bits = TEndIndex - TStartIndex + 1;
    static constexpr u8 shift = INSTRUCTION_WIDTH - TEndIndex - 1;
    static constexpr u32 mask = (1ULL << bits) - 1;
};

template <u8 TStartIndex, u8 TEndIndex>
struct ExtendedOpcodeField : Field<TStartIndex, TEndIndex, u16> {
    static constexpr bool is_extended_opcode = true;
};

template <u8 TStart, u8 TEnd>
struct GPRField : Field<TStart, TEnd, u8> {
    static constexpr bool is_operand_field = true;
    static constexpr bool is_register_field = true;

    static constexpr Operand
    make(u8 raw, Operand::Access access) {
        return Operand{//
            .value = Register::GPR{raw},
            .access = access};
    }
};

template <u8 TStart, u8 TEnd>
struct FPRField : Field<TStart, TEnd, u8> {
    static constexpr bool is_operand_field = true;
    static constexpr bool is_register_field = true;

    static constexpr Operand
    make(u8 raw, Operand::Access access) {
        return Operand{//
            .value = Register::FPR{raw},
            .access = access};
    }
};

template <u8 TStart, u8 TEnd>
struct CRField : Field<TStart, TEnd, u8> {
    static constexpr bool is_operand_field = true;
    static constexpr bool is_register_field = true;

    static constexpr Operand
    make(u8 raw, Operand::Access access) {
        return Operand{//
            .value = Register::CR{raw},
            .access = access};
    }
};

template <u8 TStart, u8 TEnd>
struct SPRField : Field<TStart, TEnd, u16> {
    static constexpr bool is_operand_field = true;
    static constexpr bool is_register_field = true;

    static constexpr Operand
    make(u16 raw, Operand::Access access) {
        return Operand{//
            .value = Register::SPR{static_cast<u16>(swap_order(raw))},
            .access = access};
    }
};

template <u8 TStart, u8 TEnd, typename TDataType = s32>
struct ImmediateField : Field<TStart, TEnd, TDataType> {
    static constexpr bool is_operand_field = true;

    static constexpr Operand
    make(TDataType raw, Operand::Access access) {
        return Operand{//
            .value = Operand::Immediate{raw},
            .access = access};
    }
};

template <u8 TStart, u8 TEnd>
struct SplitImmediateField : Field<TStart, TEnd, u32> {
    static constexpr bool is_operand_field = true;

    static constexpr Operand
    make(u32 raw, Operand::Access access) {
        return Operand{//
            .value = Operand::Immediate{static_cast<s32>(swap_order(raw))},
            .access = access};
    }
};

template <u8 TStart, u8 TEnd, typename TDataType = s16>
struct BranchDestinationField : Field<TStart, TEnd, TDataType> {
    static constexpr bool is_operand_field = true;

    static constexpr Operand
    make(TDataType raw, Operand::Access access) {
        return Operand{//
            .value = Operand::BranchDestination{static_cast<u32>(raw) << 2},
            .access = access};
    }
};

template <u8 TStart, u8 TEnd>
struct BranchOptionsField : Field<TStart, TEnd, u8> {
    static constexpr bool is_operand_field = true;

    static constexpr Operand
    make(u8 raw, Operand::Access access) {
        return Operand{//
            .value = Operand::BranchOptions{raw},
            .access = access};
    }
};

template <u8 TStart, u8 TEnd, Operand::Behavior TBehavior>
struct BehaviorField : Field<TStart, TEnd, u8> {
    static constexpr bool is_behavior_field = true;
    static constexpr auto behavior = TBehavior;
};

} // namespace Revo::PPC

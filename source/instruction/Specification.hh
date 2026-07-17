#pragma once

#include "instruction/Layout.hh"
#include "instruction/Mnemonic.hh"

namespace Revo::InstructionSpecification {

template <Mnemonic TMnemonic>
struct Specification;

template <>
struct Specification<Mnemonic::STW> {
    using Layout = InstructionLayout::DForm::Impl_RS_RA_D;
    static constexpr u8 opcd = 36;
};

template <>
struct Specification<Mnemonic::LWZ> {
    using Layout = InstructionLayout::DForm::Impl_RT_RA_D;
    static constexpr u8 opcd = 32;
};

template <>
struct Specification<Mnemonic::BCLR> {
    using Layout = InstructionLayout::XLForm::Impl_BO_BI_BH_XO_LK;
    static constexpr u8 opcd = 19;
    static constexpr u16 xo = 16;
};

template <>
struct Specification<Mnemonic::ADD> {
    using Layout = InstructionLayout::XOForm::Impl_RT_RA_RB_OE_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 266;
};

template <>
struct Specification<Mnemonic::SUBF> {
    using Layout = InstructionLayout::XOForm::Impl_RT_RA_RB_OE_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 40;
};

template <>
struct Specification<Mnemonic::MULLW> {
    using Layout = InstructionLayout::XOForm::Impl_RT_RA_RB_OE_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 235;
};

template <>
struct Specification<Mnemonic::ANDI_RC> {
    using Layout = InstructionLayout::DForm::Impl_RS_RA_UI;
    static constexpr u8 opcd = 28;
    static constexpr auto implied_behaviours = std::array{Operand::Behavior::Record};
};

template <>
struct Specification<Mnemonic::XOR> {
    using Layout = InstructionLayout::XForm::Impl_RS_RA_RB_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 316;
};

template <>
struct Specification<Mnemonic::OR> {
    using Layout = InstructionLayout::XForm::Impl_RS_RA_RB_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 444;
};

template <>
struct Specification<Mnemonic::RLWINM> {
    using Layout = InstructionLayout::MForm::Impl_RS_RA_SH_MB_ME_Rc;
    static constexpr u8 opcd = 21;
};

template <>
struct Specification<Mnemonic::ADDI> {
    using Layout = InstructionLayout::DForm::Impl_RT_RA_SI;
    static constexpr u8 opcd = 14;
};

template <>
struct Specification<Mnemonic::CMPI> {
    using Layout = InstructionLayout::DForm::Impl_BF_L_RA_SI;
    static constexpr u8 opcd = 11;
};

template <>
struct Specification<Mnemonic::BC> {
    using Layout = InstructionLayout::BForm::Impl;
    static constexpr u8 opcd = 16;
};

} // namespace Revo::InstructionSpecification

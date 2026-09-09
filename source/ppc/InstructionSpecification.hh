#pragma once

#include "ppc/Concepts.hh"
#include "ppc/Form.hh"
#include "ppc/Mnemonic.hh"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <meta>

namespace Revo::PPC {

template <Mnemonic TMnemonic>
struct InstructionSpecification;

/// \cond
template <>
struct InstructionSpecification<Mnemonic::STW> {
    static constexpr auto layout = ^^Form::DForm::Impl_RS_RA_D;
    static constexpr auto zeroable_field = ^^Form::DForm::RA;
    static constexpr u8 opcd = 36;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::LWZ> {
    static constexpr auto layout = ^^Form::DForm::Impl_RT_RA_D;
    static constexpr auto zeroable_field = ^^Form::DForm::RA;
    static constexpr u8 opcd = 32;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RT, Operand::Access::Write},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::BCLR> {
    static constexpr auto layout = ^^Form::XLForm::Impl_BO_BI_BH_XO_LK;
    static constexpr u8 opcd = 19;
    static constexpr u16 xo = 16;
    static constexpr auto indirect_branch_source = Register::SPR::LR;
};

template <>
struct InstructionSpecification<Mnemonic::ADD> {
    static constexpr auto layout = ^^Form::XOForm::Impl_RT_RA_RB_OE_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 266;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::XOForm::RT, Operand::Access::Write},
        FieldAccess{^^Form::XOForm::RA, Operand::Access::Read},
        FieldAccess{^^Form::XOForm::RB, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::SUBF> {
    static constexpr auto layout = ^^Form::XOForm::Impl_RT_RA_RB_OE_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 40;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::XOForm::RT, Operand::Access::Write},
        FieldAccess{^^Form::XOForm::RA, Operand::Access::Read},
        FieldAccess{^^Form::XOForm::RB, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::MULLW> {
    static constexpr auto layout = ^^Form::XOForm::Impl_RT_RA_RB_OE_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 235;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::XOForm::RT, Operand::Access::Write},
        FieldAccess{^^Form::XOForm::RA, Operand::Access::Read},
        FieldAccess{^^Form::XOForm::RB, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::ANDI_RC> {
    static constexpr auto layout = ^^Form::DForm::Impl_RS_RA_UI;
    static constexpr u8 opcd = 28;
    static constexpr auto implied_behaviors = Operand::Behavior::Record;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Write}};
};

template <>
struct InstructionSpecification<Mnemonic::XOR> {
    static constexpr auto layout = ^^Form::XForm::Impl_RS_RA_RB_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 316;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::XForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::XForm::RA, Operand::Access::Write},
        FieldAccess{^^Form::XForm::RB, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::OR> {
    static constexpr auto layout = ^^Form::XForm::Impl_RS_RA_RB_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 444;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::XForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::XForm::RA, Operand::Access::Write},
        FieldAccess{^^Form::XForm::RB, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::RLWINM> {
    static constexpr auto layout = ^^Form::MForm::Impl_RS_RA_SH_MB_ME_Rc;
    static constexpr u8 opcd = 21;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::MForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::MForm::RA, Operand::Access::Write}};
};

template <>
struct InstructionSpecification<Mnemonic::ADDI> {
    static constexpr auto layout = ^^Form::DForm::Impl_RT_RA_SI;
    static constexpr auto zeroable_field = ^^Form::DForm::RA;
    static constexpr u8 opcd = 14;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RT, Operand::Access::Write},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::CMPI> {
    static constexpr auto layout = ^^Form::DForm::Impl_BF_L_RA_SI;
    static constexpr u8 opcd = 11;
    static constexpr std::array constants{//
        FieldConstant{^^Form::DForm::L, 0}};
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::BF, Operand::Access::Write},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::BC> {
    static constexpr auto layout = ^^Form::BForm::Impl;
    static constexpr u8 opcd = 16;
};

template <>
struct InstructionSpecification<Mnemonic::B> {
    static constexpr auto layout = ^^Form::IForm::Impl;
    static constexpr u8 opcd = 18;
};

template <>
struct InstructionSpecification<Mnemonic::CMP> {
    static constexpr auto layout = ^^Form::XForm::Impl_BF_L10_RA_RB_XO;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 0;
    static constexpr std::array constants{//
        FieldConstant{^^Form::XForm::L10, 0}};
    static constexpr std::array accesses{//
        FieldAccess{^^Form::XForm::BF, Operand::Access::Write},
        FieldAccess{^^Form::XForm::RA, Operand::Access::Read},
        FieldAccess{^^Form::XForm::RB, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::MTSPR> {
    static constexpr auto layout = ^^Form::XFXForm::Impl_RS_SPR_XO;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 467;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::XFXForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::XFXForm::SPR, Operand::Access::Write}};
};

template <>
struct InstructionSpecification<Mnemonic::ADDIC_RC> {
    static constexpr auto layout = ^^Form::DForm::Impl_RT_RA_SI;
    static constexpr u8 opcd = 13;
    static constexpr auto implied_behaviors = Operand::Behavior::Record;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RT, Operand::Access::Write},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::BCCTR> {
    static constexpr auto layout = ^^Form::XLForm::Impl_BO_BI_BH_XO_LK;
    static constexpr u8 opcd = 19;
    static constexpr u16 xo = 528;
    static constexpr auto indirect_branch_source = Register::SPR::CTR;
};

template <>
struct InstructionSpecification<Mnemonic::MFSPR> {
    static constexpr auto layout = ^^Form::XFXForm::Impl_RT_SPR_XO;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 339;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::XFXForm::RT, Operand::Access::Write},
        FieldAccess{^^Form::XFXForm::SPR, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::STWU> {
    static constexpr auto layout = ^^Form::DForm::Impl_RS_RA_D;
    static constexpr u8 opcd = 37;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::DForm::RA, Operand::Access::ReadWrite}};
};

template <>
struct InstructionSpecification<Mnemonic::LBZ> {
    static constexpr auto layout = ^^Form::DForm::Impl_RT_RA_D;
    static constexpr auto zeroable_field = ^^Form::DForm::RA;
    static constexpr u8 opcd = 34;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RT, Operand::Access::Write},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::ADDIS> {
    static constexpr auto layout = ^^Form::DForm::Impl_RT_RA_SI;
    static constexpr auto zeroable_field = ^^Form::DForm::RA;
    static constexpr u8 opcd = 15;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RT, Operand::Access::Write},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::MULLI> {
    static constexpr auto layout = ^^Form::DForm::Impl_RT_RA_SI;
    static constexpr u8 opcd = 7;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RT, Operand::Access::Write},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::CMPL> {
    static constexpr auto layout = ^^Form::XForm::Impl_BF_L10_RA_RB_XO;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 32;
    static constexpr std::array constants{//
        FieldConstant{^^Form::XForm::L10, 0}};
    static constexpr std::array accesses{//
        FieldAccess{^^Form::XForm::BF, Operand::Access::Write},
        FieldAccess{^^Form::XForm::RA, Operand::Access::Read},
        FieldAccess{^^Form::XForm::RB, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::CRXOR> {
    static constexpr auto layout = ^^Form::XLForm::Impl_BT_BA_BB_XO;
    static constexpr u8 opcd = 19;
    static constexpr u16 xo = 193;
};

template <>
struct InstructionSpecification<Mnemonic::STB> {
    static constexpr auto layout = ^^Form::DForm::Impl_RS_RA_D;
    static constexpr auto zeroable_field = ^^Form::DForm::RA;
    static constexpr u8 opcd = 38;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Read}};
};

template <>
struct InstructionSpecification<Mnemonic::ORI> {
    static constexpr auto layout = ^^Form::DForm::Impl_RS_RA_UI;
    static constexpr u8 opcd = 24;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Write}};
};

template <>
struct InstructionSpecification<Mnemonic::XORI> {
    static constexpr auto layout = ^^Form::DForm::Impl_RS_RA_UI;
    static constexpr u8 opcd = 26;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::DForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::DForm::RA, Operand::Access::Write}};
};

template <>
struct InstructionSpecification<Mnemonic::CNTLZW> {
    static constexpr auto layout = ^^Form::XForm::Impl_RS_RA_XO_Rc;
    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 26;
    static constexpr std::array accesses{//
        FieldAccess{^^Form::XForm::RS, Operand::Access::Read},
        FieldAccess{^^Form::XForm::RA, Operand::Access::Write}};
};
/// \endcond

} // namespace Revo::PPC

#pragma once

#include "instruction/Layout.hh"
#include "instruction/Mnemonic.hh"

#include <concepts>

namespace Revo {

using namespace InstructionLayout;

/**
 * @brief API for whether a specification has an extended opcode.
 *
 * Instructions can contain a primary opcode and an optional extended opcode,
 * used to identify what operation to carry out on the CPU.
 *
 * xo is a varaible that provides that extended opcode, and HasExtendedOpcode
 * is an API that returns whether a given Specification defines an extended
 * opcode.
 *
 * @tparam TSpecification The Specification checked.
 *
 * @return **bool** Whether the given specification has extended opcode.
 */
template <typename TSpecification>
concept HasExtendedOpcode = requires { TSpecification::xo; };

/**
 * @brief API for whether a specification has any implied behaviours.
 *
 * Instruction layouts may contain fields that change CPU flags, but
 * instructions themselves can additionally and implicitly change CPU flags
 * without a relevant field in their layout.
 *
 * This behaviour is physically baked into the CPUs themselves and thus needs to
 * be reimplemented, which is done using implied_behaviors.
 *
 * implied_behaviors provides additional flags outside the context of layout
 * fields, and HasImpliedBehaviors is the API that returns whether a given
 * Specification defines any implicit behaviours.
 *
 * @tparam TSpecification The Specification checked.
 *
 * @return **bool** Whether the given specification has any implied behaviours.
 */
template <typename TSpecification>
concept HasImpliedBehaviors = requires { TSpecification::implied_behaviors; };

template <typename TSpecification>
concept HasFieldConstants = requires { typename TSpecification::Constants; };

template <typename TSpecification>
concept HasZeroableField = requires { typename TSpecification::ZeroableField; };

template <typename TSpecification>
concept HasAccesses = requires { typename TSpecification::Accesses; };

/**
 * @brief API for whether a specification field is zeroable.
 *
 * Depending on the instruction GPR fields can be zeroable, meaning that a value
 * of value of 0 corresponds to a decimal 0 instead of register r0.
 *
 * ZeroableField specifies what field in an instruction's layout is zeroable
 * and IsZeroableField is an API that returns whether a given Specification
 * field is zeroable.
 *
 * @tparam TSpecification The Specification checked.
 * @tparam TField The checked field.
 *
 * @return **bool** Whether the given field was zeroable.
 */
template <typename TSpecification, typename TField>
concept IsZeroableField = requires {
    requires std::same_as<typename TSpecification::ZeroableField, TField>;
};

/**
 * @brief API for whether a specification field is a destination.
 *
 * Specific instruction fields in PPC can be both read from and written to,
 * depending on the instruction. For example, in `xor`, RA is the destination
 * register, but in `add`, it's the first source register.
 *
 * This behaviour is physically baked into the CPUs themselves and thus needs to
 * be reimplemented, which is done using DestinationField.
 *
 * DestinationField specifies what field is the destination, and
 * IsDestinationField is an API that returns whether a given Specification field
 * is a destination.
 *
 * @tparam TSpecification The Specification checked.
 * @tparam TField The checked field.
 *
 * @return **bool** Whether the given field was a destination.
 */
template <typename TSpecification, typename TField>
concept IsDestinationField = requires {
    requires std::same_as<typename TSpecification::DestinationField, TField>;
};

template <typename TField, Operand::Access TAccess>
struct FieldAccess {};

template <typename... TFieldAccesses>
struct FieldAccesses {};

template <typename TField, u32 TValue>
struct FieldConstant {};

template <typename... TFieldConstants>
struct FieldConstants {};

/**
 * @brief Specification for an instruction.
 *
 * Every Mnemonic **must** have a template specialization of this struct, which
 * provides the information needed for the decoder to work correctly.
 *
 * An instruction **must** provide:
 * - A primary opcode
 *     ```c++
 *     static constexpr u8 opcd = 36;
 *     ```
 * - An instruction layout
 *     ```c++
 *     using Layout = XForm::Impl_RS_RA_RB_XO_Rc;
 *     ```
 * - (If an instruction layout references a register field) Field access types:
 *     ```c++
 *     using Accesses = FieldAccesses< //
           FieldAccess<DForm::RS, Operand::Access::Read>,
           FieldAccess<DForm::RA, Operand::Access::Read>>;
 *     ```
 *
 * In addition to the required fields, an instruction may also provide:
 * - An extended opcode
 *     ```c++
 *     static constexpr u16 xo = 266;
 *     ```
 * - A zeroable field
 *     ```c++
 *     using ZeroableField = DForm::RA;
 *     ```
 * - A destination field
 *     ```c++
 *     using DestinationField = DForm::RA;
 *     ```
 *
 * The given values **must** be derived using by the PPC ISA.
 *
 * @tparam TMnemonic The instructoin mnemonic to be implemented.
 */
template <Mnemonic TMnemonic>
struct InstructionSpecification;

/// \cond
template <>
struct InstructionSpecification<Mnemonic::STW> {
    using Layout = DForm::Impl_RS_RA_D;
    using ZeroableField = DForm::RA;
    using Accesses = FieldAccesses< //
        FieldAccess<DForm::RS, Operand::Access::Read>,
        FieldAccess<DForm::RA, Operand::Access::Read>>;

    static constexpr u8 opcd = 36;
};

template <>
struct InstructionSpecification<Mnemonic::LWZ> {
    using Layout = DForm::Impl_RT_RA_D;
    using ZeroableField = DForm::RA;
    using Accesses = FieldAccesses< //
        FieldAccess<DForm::RT, Operand::Access::Write>,
        FieldAccess<DForm::RA, Operand::Access::Read>>;

    static constexpr u8 opcd = 32;
};

template <>
struct InstructionSpecification<Mnemonic::BCLR> {
    using Layout = XLForm::Impl_BO_BI_BH_XO_LK;

    static constexpr u8 opcd = 19;
    static constexpr u16 xo = 16;
};

template <>
struct InstructionSpecification<Mnemonic::ADD> {
    using Layout = XOForm::Impl_RT_RA_RB_OE_XO_Rc;
    using Accesses = FieldAccesses< //
        FieldAccess<XOForm::RT, Operand::Access::Write>,
        FieldAccess<XOForm::RA, Operand::Access::Read>,
        FieldAccess<XOForm::RB, Operand::Access::Read>>;

    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 266;
};

template <>
struct InstructionSpecification<Mnemonic::SUBF> {
    using Layout = XOForm::Impl_RT_RA_RB_OE_XO_Rc;
    using Accesses = FieldAccesses< //
        FieldAccess<XOForm::RT, Operand::Access::Write>,
        FieldAccess<XOForm::RA, Operand::Access::Read>,
        FieldAccess<XOForm::RB, Operand::Access::Read>>;

    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 40;
};

template <>
struct InstructionSpecification<Mnemonic::MULLW> {
    using Layout = XOForm::Impl_RT_RA_RB_OE_XO_Rc;
    using Accesses = FieldAccesses< //
        FieldAccess<XOForm::RT, Operand::Access::Write>,
        FieldAccess<XOForm::RA, Operand::Access::Read>,
        FieldAccess<XOForm::RB, Operand::Access::Read>>;

    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 235;
};

template <>
struct InstructionSpecification<Mnemonic::ANDI_RC> {
    using Layout = DForm::Impl_RS_RA_UI;
    using DestinationField = DForm::RA;
    using Accesses = FieldAccesses< //
        FieldAccess<DForm::RS, Operand::Access::Read>,
        FieldAccess<DForm::RA, Operand::Access::Write>>;

    static constexpr u8 opcd = 28;
    static constexpr auto implied_behaviors = Operand::Behavior::Record;
};

template <>
struct InstructionSpecification<Mnemonic::XOR> {
    using Layout = XForm::Impl_RS_RA_RB_XO_Rc;
    using DestinationField = XForm::RA;
    using Accesses = FieldAccesses< //
        FieldAccess<XForm::RS, Operand::Access::Read>,
        FieldAccess<XForm::RA, Operand::Access::Write>,
        FieldAccess<XForm::RB, Operand::Access::Read>>;

    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 316;
};

template <>
struct InstructionSpecification<Mnemonic::OR> {
    using Layout = XForm::Impl_RS_RA_RB_XO_Rc;
    using DestinationField = XForm::RA;
    using Accesses = FieldAccesses< //
        FieldAccess<XForm::RS, Operand::Access::Read>,
        FieldAccess<XForm::RA, Operand::Access::Write>,
        FieldAccess<XForm::RB, Operand::Access::Read>>;

    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 444;
};

template <>
struct InstructionSpecification<Mnemonic::RLWINM> {
    using Layout = MForm::Impl_RS_RA_SH_MB_ME_Rc;
    using DestinationField = MForm::RA;
    using Accesses = FieldAccesses< //
        FieldAccess<MForm::RS, Operand::Access::Read>,
        FieldAccess<MForm::RA, Operand::Access::Write>>;

    static constexpr u8 opcd = 21;
};

template <>
struct InstructionSpecification<Mnemonic::ADDI> {
    using Layout = DForm::Impl_RT_RA_SI;
    using ZeroableField = DForm::RA;
    using Accesses = FieldAccesses< //
        FieldAccess<DForm::RT, Operand::Access::Write>,
        FieldAccess<DForm::RA, Operand::Access::Read>>;

    static constexpr u8 opcd = 14;
};

template <>
struct InstructionSpecification<Mnemonic::CMPI> {
    using Layout = DForm::Impl_BF_L_RA_SI;
    using Constants = FieldConstants<FieldConstant<DForm::L, 0>>;
    using Accesses = FieldAccesses< //
        FieldAccess<DForm::BF, Operand::Access::Write>,
        FieldAccess<DForm::RA, Operand::Access::Read>>;

    static constexpr u8 opcd = 11;
};

template <>
struct InstructionSpecification<Mnemonic::BC> {
    using Layout = BForm::Impl;

    static constexpr u8 opcd = 16;
};

template <>
struct InstructionSpecification<Mnemonic::B> {
    using Layout = IForm::Impl;

    static constexpr u8 opcd = 18;
};

template <>
struct InstructionSpecification<Mnemonic::CMP> {
    using Layout = XForm::Impl_BF_L10_RA_RB_XO;
    using Constants = FieldConstants<FieldConstant<XForm::L10, 0>>;
    using Accesses = FieldAccesses< //
        FieldAccess<XForm::BF, Operand::Access::Write>,
        FieldAccess<XForm::RA, Operand::Access::Read>,
        FieldAccess<XForm::RB, Operand::Access::Read>>;

    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 0;
};

template <>
struct InstructionSpecification<Mnemonic::MTSPR> {
    using Layout = XFXForm::Impl_RS_SPR_XO;
    using Accesses = FieldAccesses< //
        FieldAccess<XFXForm::RS, Operand::Access::Read>,
        FieldAccess<XFXForm::SPR, Operand::Access::Write>>;

    static constexpr u8 opcd = 31;
    static constexpr u16 xo = 467;
};

template <>
struct InstructionSpecification<Mnemonic::ADDIC_RC> {
    using Layout = DForm::Impl_RT_RA_SI;
    using Accesses = FieldAccesses< //
        FieldAccess<DForm::RT, Operand::Access::Write>,
        FieldAccess<DForm::RA, Operand::Access::Read>>;

    static constexpr u8 opcd = 13;
    static constexpr auto implied_behaviors = Operand::Behavior::Record;
};
/// \endcond

} // namespace Revo

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

} // namespace Revo::InstructionSpecification

#pragma once

#include "instruction/Layout.hh"
#include "instruction/Mnemonic.hh"

namespace Revo::InstructionSpecification {

template <Mnemonic TMnemonic>
struct Specification;

template <>
struct Specification<Mnemonic::STW> {
    using Layout = InstructionLayout::DForm::Impl_RS_RA_D;
};

template <>
struct Specification<Mnemonic::LWZ> {
    using Layout = InstructionLayout::DForm::Impl_RT_RA_D;
};

template <>
struct Specification<Mnemonic::BCLR> {
    using Layout = InstructionLayout::XLForm::Impl_BO_BI_BH_XO_LK;
};

} // namespace Revo::InstructionSpecification

#pragma once

#include "instruction/Mnemonic.hh"
#include "instruction/Layout.hh"

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
struct Specification<Mnemonic::BLR> {
    using Layout = InstructionLayout::XLForm::Impl_XO;
};

} // namespace Revo::InstructionSpecification

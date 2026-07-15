#pragma once

#include "instruction/Mnemonic.hh"
#include "instruction/Format.hh"

namespace Revo::Instruction::Specification {

template <Mnemonic TMnemonic>
struct InstructionSpecification;

template <>
struct InstructionSpecification<Mnemonic::STW> {
    using Format = InstructionFormat::DForm::Impl_RS_RA_D;
}

} // namespace Revo::Instruction::Specification

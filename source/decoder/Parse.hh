#pragma once

#include "decoder/Instruction.hh"
#include "decoder/Mnemonic.hh"
#include "decoder/Format.hh"

// todo: force compile-time check of mnemonic implementations

namespace Revo::Decoder::Parse {

template <Mnemonic TMnemonic>
[[nodiscard]] constexpr DecodedInstruction
decode(Instruction instruction);

} // namespace Revo::Decoder::Parse

#pragma once

#include "decode/Types.hh"
#include "elf/Types.hh"
#include "ppc/Mnemonic.hh"

#include <expected>
#include <span>
#include <string>
#include <vector>

namespace Revo::Decode {

[[nodiscard]] std::expected<std::vector<Function>, std::string>
decode(std::span<const ELF::Function> functions);

namespace Impl {

// Decoding steps
[[nodiscard]] std::expected<Function, std::string>
decode_function(const ELF::Function& function);

[[nodiscard]] std::expected<Instruction, std::string>
parse(u32 raw, u32 address);

template <PPC::Mnemonic TMnemonic>
[[nodiscard]] constexpr Instruction
make_instruction(u32 raw, u32 address);

} // namespace Impl

} // namespace Revo::Decode

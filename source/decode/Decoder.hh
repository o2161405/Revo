#pragma once

#include "decode/Types.hh"
#include "elf/Parser.hh"
#include "ppc/Instruction.hh"
#include "ppc/InstructionSpecification.hh"
#include "ppc/Mnemonic.hh"
#include "ppc/Operand.hh"

#include <expected>
#include <meta>
#include <vector>

namespace Revo {

class Decoder {
public:
    struct Result {
        std::vector<Decode::Function> functions;
    };

    [[nodiscard]] static std::expected<Result, std::string>
    decode(const std::vector<ELF::Function>& functions);

private:
    // Decoding steps
    [[nodiscard]] static std::expected<Decode::Instruction, std::string>
    parse(PPC::Instruction instruction, u32 address);

    template <PPC::Mnemonic TMnemonic>
    [[nodiscard]] static constexpr Decode::Instruction
    make_instruction(PPC::Instruction instruction, u32 address);
};

} // namespace Revo

#pragma once

#include "decode/Types.hh"
#include "elf/Parser.hh"
#include "ppc/Instruction.hh"
#include "ppc/Mnemonic.hh"
#include "ppc/Operand.hh"
#include "ppc/Specification.hh"

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
    // --- Decoding steps ---
    [[nodiscard]] static std::expected<Decode::Instruction, std::string>
    decode_instruction(PPC::Instruction instruction, u32 address);

    template <PPC::Mnemonic TMnemonic>
    [[nodiscard]] static constexpr Decode::Instruction
    decode(PPC::Instruction instruction, u32 address);

    // --- Utility functions ---
    [[nodiscard]] static consteval bool
    is_register(PPC::Operand::Type type) {
        return type == PPC::Operand::Type::GPR || //
            type == PPC::Operand::Type::FPR || //
            type == PPC::Operand::Type::SPR || //
            type == PPC::Operand::Type::CR;
    }

    template <typename... TFields, u32... TAccesses>
    [[nodiscard]] static constexpr bool
    validate_constants(PPC::Instruction instruction,
        PPC::FieldConstants<PPC::FieldConstant<TFields, TAccesses>...>) {
        return ((instruction.get<TFields>() == TAccesses) && ...);
    }

    template <typename TSpecification, typename TField>
    [[nodiscard]] static consteval PPC::Operand::Access
    get_access_type();
};

} // namespace Revo

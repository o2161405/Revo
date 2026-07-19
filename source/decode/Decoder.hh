#pragma once

#include "elf/Function.hh"
#include "elf/Parser.hh"
#include "instruction/Instruction.hh"
#include "instruction/Mnemonic.hh"
#include "instruction/Operand.hh"
#include "instruction/Specification.hh"

#include <expected>
#include <meta>

namespace Revo {

class Decoder {
public:
    using Function = ELF::FunctionImpl<DecodedInstruction>;

    [[nodiscard]] static std::expected<Decoder, std::string>
    decode(const std::vector<ELF::Parser::Function>& functions);

private:
    Decoder() = default;

    // --- Decoding steps ---
    [[nodiscard]] static std::expected<DecodedInstruction, std::string>
    decode_instruction(Instruction instruction, u32 address);

    template <Mnemonic TMnemonic>
    [[nodiscard]] static constexpr DecodedInstruction
    decode(Instruction instruction, u32 address);

    // --- Utility functions ---
    [[nodiscard]] static consteval bool
    is_register(Operand::Type type) {
        return type == Operand::Type::GPR || type == Operand::Type::FPR ||
            type == Operand::Type::SPR || type == Operand::Type::CR;
    }

    template <typename... TFields, u32... TValues>
    [[nodiscard]] static constexpr bool
    valid_field_constants(
        Instruction instruction, FieldConstants<FieldConstant<TFields, TValues>...>) {
        return ((instruction.get<TFields>() == TValues) && ...);
    }

    template <typename TSpecification, typename TField>
    [[nodiscard]] static consteval Operand::Access
    get_access_type();

    // --- Member variables ---
    std::vector<Function> mFunctions;
};

} // namespace Revo

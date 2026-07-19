#pragma once

#include "file/ELF.hh"
#include "file/Function.hh"
#include "instruction/Instruction.hh"
#include "instruction/Mnemonic.hh"
#include "instruction/Operand.hh"

namespace Revo {

class Decoder {
public:
    using Function = FunctionImpl<DecodedInstruction, ELF::RelativeOffset, ELF::Rela>;

    [[nodiscard]] static std::expected<Decoder, std::string>
    decode(const ELF& elf);

private:
    Decoder() = default;

    template <typename TField>
    static constexpr bool is_extended_opcode_v = requires { requires TField::is_extended_opcode; };

    template <typename TField>
    static constexpr Operand::Role default_role_v = [] {
        if constexpr (requires { TField::role; }) {
            return TField::role;
        }
        else if constexpr (TField::operand_type == Operand::Type::GPR ||
            TField::operand_type == Operand::Type::FPR ||
            TField::operand_type == Operand::Type::CR) {
            return Operand::Role::Read;
        }
        else {
            return Operand::Role::None;
        }
    }();

    // --- Decoding steps ---
    template <Mnemonic TMnemonic>
    [[nodiscard]] static constexpr DecodedInstruction
    decode(Instruction instruction);

    // --- Utility functions ---
    [[nodiscard]] static std::expected<DecodedInstruction, std::string>
    decode_instruction(Instruction instruction);

    // --- Member variables ---
    std::vector<Function> mFunctions;
};

} // namespace Revo

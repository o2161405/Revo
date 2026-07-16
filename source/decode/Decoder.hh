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

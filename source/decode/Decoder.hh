#pragma once

#include "file/ELF.hh"
#include "instruction/Instruction.hh"
#include "instruction/Mnemonic.hh"
#include "instruction/Operand.hh"

namespace Revo {

class Decoder {
public:
    // todo: refactor ELF::Function and this to be a Result struct instead
    // this is just a copy of ELF::Function with one variable changed and i dont like it
    struct Function {
        std::vector<DecodedInstruction> instructions;
        std::flat_map<ELF::RelativeOffset, std::vector<ELF::Rela>> relocations;
        u32 offset;
        u32 size;
    };

    [[nodiscard]] static std::expected<Decoder, std::string>
    decode(const ELF& elf);

private:
    Decoder() = default;

    // --- Decoding steps ---
    template <Mnemonic TMnemonic>
    [[nodiscard]] static constexpr DecodedInstruction
    decode(Instruction instruction);

    // --- Utility functions ---
    [[nodiscard]] static std::expected<DecodedInstruction, std::string>
    decode_primary(Instruction instruction);

    [[nodiscard]] static std::expected<DecodedInstruction, std::string>
    decode_extended(u8 opcd, Instruction instruction);

    // --- Member variables ---
    std::vector<Function> mFunctions;
};

} // namespace Revo

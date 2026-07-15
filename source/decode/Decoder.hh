#pragma once

#include "decode/Instruction.hh"
#include "decode/Mnemonic.hh"
#include "decode/Operand.hh"
#include "file/ELF.hh"

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

    // --- Member variables ---
    std::vector<Function> mFunctions;
};

} // namespace Revo

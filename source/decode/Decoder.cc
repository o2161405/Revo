#include "Decoder.hh"

#include "instruction/Layout.hh"
#include "instruction/Specification.hh"

#include <meta>

namespace Revo {

std::expected<Decoder, std::string>
Decoder::decode(const ELF& elf) {
    Decoder decoder;

    for (const auto& function : elf.functions()) {
        Function decoded{//
            .instructions = {},
            .relocations = function.relocations,
            .offset = function.offset,
            .size = function.size};

        decoded.instructions.reserve(function.instructions.size());

        for (auto [index, raw] : std::views::enumerate(function.instructions)) {
            auto result = decode_primary(Instruction{static_cast<u32>(raw)});
            if (!result) {
                return std::unexpected(std::format(
                    "{} at {:#x}", result.error(), function.offset + index * sizeof(u32)));
            }
            decoded.instructions.push_back(*result);
        }

        decoder.mFunctions.push_back(std::move(decoded));
    }

    Console::info("Decoded {} functions", decoder.mFunctions.size());
    return decoder;
}

template <Mnemonic TMnemonic>
[[nodiscard]] constexpr DecodedInstruction
Decoder::decode(Instruction instruction) {
    using Layout = InstructionSpecification::Specification<TMnemonic>::Layout;

    DecodedInstruction decoded_instruction{.mnemonic = TMnemonic};
    auto index{0uz};

    // todo: is this possible without dealiasing? it makes printing disassembly harder
    // if we ever want to do that
    template for (constexpr auto field :
        std::define_static_array(std::meta::template_arguments_of(std::meta::dealias(^^Layout)))) {
        using TField = [:field:];
        if constexpr (TField::operand_type != Operand::Type::None) {
            decoded_instruction.operands[index++] =
                Operand::get<TField::operand_type>(instruction.get<TField>());
        }
        else if constexpr (TField::operand_behaviour != Operand::Behavior::None) {
            decoded_instruction.behaviors[std::to_underlying(TField::operand_behaviour) - 1] =
                instruction.get<TField>() != 0;
        }
    }

    return decoded_instruction;
}

std::expected<DecodedInstruction, std::string>
Decoder::decode_primary(Instruction instruction) {
    const auto opcd = instruction.get<InstructionLayout::OPCD>();

    switch (opcd) {
    case 32: return decode<Mnemonic::LWZ>(instruction);
    case 36: return decode<Mnemonic::STW>(instruction);
    case 19:
    case 31: return decode_extended(opcd, instruction);
    default: return std::unexpected(std::format("unimplemented primary opcode ({})", opcd));
    }
}

std::expected<DecodedInstruction, std::string>
Decoder::decode_extended(u8 opcd, Instruction instruction) {
    const auto xo = instruction.get<InstructionLayout::XLForm::XO>();

    switch (opcd) {
    case 19:
        switch (xo) {
        case 16: return decode<Mnemonic::BLR>(instruction);
        }
        break;
    }

    return std::unexpected(
        std::format("unimplemented extended opcode {} (primary opcode {})", xo, opcd));
}

} // namespace Revo

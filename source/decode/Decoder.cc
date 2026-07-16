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
            auto result = decode_instruction(Instruction{static_cast<u32>(raw)});
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
            decoded_instruction.operands[index++] = Operand::get<TField::operand_type>(
                instruction.get<TField>());
        }
        else if constexpr (TField::operand_behaviour != Operand::Behavior::None) {
            decoded_instruction.behaviors[std::to_underlying(TField::operand_behaviour) - 1] =
                instruction.get<TField>() != 0;
        }
    }

    return decoded_instruction;
}

std::expected<DecodedInstruction, std::string>
Decoder::decode_instruction(Instruction instruction) {
    constexpr auto get_xo_field = [](std::meta::info layout) consteval -> std::meta::info {
        for (auto field : std::meta::template_arguments_of(std::meta::dealias(layout))) {
            if (std::meta::extract<bool>(std::meta::substitute(^^Decoder::is_extended_opcode_v, {field}))) {
                return field;
            }
        }
        return {};
    };

    const auto opcd = instruction.get<InstructionLayout::OPCD>();

    template for (constexpr auto enumerator :
        std::define_static_array(std::meta::enumerators_of(^^Mnemonic))) {
        constexpr auto mnemonic = [:enumerator:];
        using Specification = InstructionSpecification::Specification<mnemonic>;
        constexpr auto xo_field = get_xo_field(^^typename Specification::Layout);

        if (opcd == Specification::opcd) {
            if constexpr (xo_field != std::meta::info{}) {
                static_assert(
                    requires { Specification::xo; },
                    "Layout has an extended opcode but the instruction specification doesn't "
                    "provide the required fields");
                using TXOField = [:xo_field:];
                if (instruction.get<TXOField>() == Specification::xo) {
                    return decode<mnemonic>(instruction);
                }
            }
            else {
                return decode<mnemonic>(instruction);
            }
        }
    }

    return std::unexpected(std::format("unimplemented opcode ({})", opcd));
}

} // namespace Revo

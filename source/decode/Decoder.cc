#include "Decoder.hh"

#include "instruction/Layout.hh"
#include "instruction/Specification.hh"

#include <algorithm>
#include <meta>

namespace Revo {

std::expected<Decoder, std::string>
Decoder::decode(const ELF& elf) {
    Decoder decoder;

    for (const auto& function : elf.functions()) {
        Function decoded_function{//
            .instructions = {},
            .relocations = function.relocations,
            .offset = function.offset,
            .size = function.size};

        decoded_function.instructions.reserve(function.instructions.size());

        for (auto [index, raw] : std::views::enumerate(function.instructions)) {
            // todo: can this error propagation be cleaner?
            auto result = decode_instruction(Instruction{static_cast<u32>(raw)});
            if (!result) {
                return std::unexpected(std::format(
                    "{} at {:#x}", result.error(), function.offset + index * sizeof(u32)));
            }
            decoded_function.instructions.push_back(*result);
        }

        decoder.mFunctions.push_back(std::move(decoded_function));
    }

    Console::info("Decoded {} functions", decoder.mFunctions.size());
    return decoder;
}

template <Mnemonic TMnemonic>
[[nodiscard]] constexpr DecodedInstruction
Decoder::decode(Instruction instruction) {
    using Specification = InstructionSpecification<TMnemonic>;
    using Layout = Specification::Layout;

    static constexpr auto fields = std::define_static_array(
        std::meta::template_arguments_of(std::meta::dealias(^^Layout)));

    if constexpr (requires { typename Specification::ZeroGPRField; }) {
        static_assert(std::ranges::contains(
                          fields, std::meta::dealias(^^typename Specification::ZeroGPRField)),
            "ZeroGPRField isn't a field of the instruction's layout");
    }

    if constexpr (requires { typename Specification::DestinationField; }) {
        static_assert(std::ranges::contains(
                          fields, std::meta::dealias(^^typename Specification::DestinationField)),
            "DestinationField isn't a field of the instruction's layout");
    }

    DecodedInstruction decoded_instruction{.mnemonic = TMnemonic};

    template for (constexpr auto field : fields) {
        using TField = [:field:];
        if constexpr (TField::operand_type != Operand::Type::None) {
            const auto value = instruction.get<TField>();

            constexpr auto role = IsDestinationField<Specification, TField> ?
                Operand::Role::Write :
                default_role_v<TField>;

            if constexpr (IsZeroableField<Specification, TField>) {
                decoded_instruction.operands.push_back(value == 0 ?
                        Operand::get<Operand::Type::Immediate>(0) :
                        Operand::get<TField::operand_type>(value, role));
            }
            else {
                decoded_instruction.operands.push_back(
                    Operand::get<TField::operand_type>(value, role));
            }
        }
        else if constexpr (TField::operand_behaviour != Operand::Behavior::None) {
            if constexpr (HasImpliedBehaviors<Specification>) {
                static_assert((TField::operand_behaviour & Specification::implied_behaviors) ==
                        Operand::Behavior::None,
                    "Operand and implied behaviour share one or more flags");
            }
            if (instruction.get<TField>() != 0) {
                decoded_instruction.behaviors |= TField::operand_behaviour;
            }
        }
    }

    if constexpr (HasImpliedBehaviors<Specification>) {
        decoded_instruction.behaviors |= Specification::implied_behaviors;
    }

    return decoded_instruction;
}

std::expected<DecodedInstruction, std::string>
Decoder::decode_instruction(Instruction instruction) {
    constexpr auto get_xo_field = [](std::meta::info layout) consteval -> std::meta::info {
        for (auto field : std::meta::template_arguments_of(std::meta::dealias(layout))) {
            if (std::meta::extract<bool>(
                    std::meta::substitute(^^Decoder::is_extended_opcode_v, {field}))) {
                return field;
            }
        }
        return {};
    };

    const auto opcd = instruction.get<InstructionLayout::OPCD>();

    template for (constexpr auto enumerator :
        std::define_static_array(std::meta::enumerators_of(^^Mnemonic))) {
        constexpr auto mnemonic = [:enumerator:];
        using Specification = InstructionSpecification<mnemonic>;
        constexpr auto xo_field = get_xo_field(^^typename Specification::Layout);

        if (opcd == Specification::opcd) {
            if constexpr (xo_field != std::meta::info{}) {
                static_assert(HasExtendedOpcode<Specification>,
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

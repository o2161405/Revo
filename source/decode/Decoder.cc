#include "Decoder.hh"

#include "instruction/Layout.hh"

#include <algorithm>
#include <meta>

namespace Revo {

std::expected<DecoderResult, std::string>
Decoder::decode(const std::vector<FunctionImpl<u32>>& functions) {
    DecoderResult result;

    for (const auto& function : functions) {
        std::vector<DecodedInstruction> instructions;
        instructions.reserve(function.instructions.size());

        for (auto [index, raw] : std::views::enumerate(function.instructions)) {
            const auto address = static_cast<u32>(function.offset + index * sizeof(u32));

            auto decoded = decode_instruction(Instruction{raw}, address);
            if (!decoded) {
                return std::unexpected(decoded.error());
            }

            instructions.push_back(*decoded);
        }

        result.functions.push_back({//
            .instructions = std::move(instructions),
            .relocations = function.relocations,
            .offset = function.offset,
            .size = function.size});
    }

    Console::info("Decoded {} functions", result.functions.size());
    return result;
}

std::expected<DecodedInstruction, std::string>
Decoder::decode_instruction(Instruction instruction, u32 address) {
    const auto opcd = instruction.get<InstructionLayout::OPCD>();
    std::optional<u32> xo;

    template for (constexpr auto enumerator :
        std::define_static_array(std::meta::enumerators_of(^^Mnemonic))) {
        constexpr auto mnemonic = [:enumerator:];
        using Specification = InstructionSpecification<mnemonic>;

        if (opcd != Specification::opcd) {
            continue;
        }

        if constexpr (HasFieldConstants<Specification>) {
            if (!valid_field_constants(instruction, typename Specification::Constants{})) {
                continue;
            }
        }

        using Layout = Specification::Layout;
        if constexpr (Layout::has_extended_opcode) {
            static_assert(HasExtendedOpcode<Specification>,
                "Layout has an extended opcode but the instruction specification doesn't "
                "provide the required fields");

            xo = Layout::extended_opcode(instruction.raw());
            if (xo != Specification::xo) {
                continue;
            }
        }

        if (Layout::uses_reserved_bits(instruction.raw())) {
            return std::unexpected(
                std::format("reserved bits set ({:#010x}) at {:#x}", instruction.raw(), address));
        }

        return decode<mnemonic>(instruction, address);
    }

    if (xo) {
        return std::unexpected(
            std::format("unimplemented opcode ({}, xo {}) at {:#x}", opcd, *xo, address));
    }

    return std::unexpected(std::format("unimplemented opcode ({}) at {:#x}", opcd, address));
}

template <Mnemonic TMnemonic>
[[nodiscard]] constexpr DecodedInstruction
Decoder::decode(Instruction instruction, u32 address) {
    using Specification = InstructionSpecification<TMnemonic>;
    using Layout = Specification::Layout;

    static constexpr auto fields = std::define_static_array(
        std::meta::template_arguments_of(std::meta::dealias(^^Layout)));

    if constexpr (HasZeroableField<Specification>) {
        static_assert(std::ranges::contains(
                          fields, std::meta::dealias(^^typename Specification::ZeroableField)),
            "ZeroableField isn't a field of the instruction's layout");
    }

    if constexpr (HasAccesses<Specification>) {
        template for (constexpr auto entry :
            std::define_static_array(std::meta::template_arguments_of(
                std::meta::dealias(^^typename Specification::Accesses)))) {
            static_assert(std::ranges::contains(fields,
                              std::meta::dealias(std::meta::template_arguments_of(entry)[0])),
                "Accesses entry references a field that isn't in the instruction's layout");
        }
    }

    DecodedInstruction decoded_instruction{.mnemonic = TMnemonic};

    template for (constexpr auto field : fields) {
        using TField = [:field:];
        if constexpr (TField::operand_type != Operand::Type::None) {
            constexpr auto access = get_access_type<Specification, TField>();
            if constexpr (is_register(TField::operand_type)) {
                static_assert(access != Operand::Access::None,
                    "Register field is missing from the specification's Accesses list");
            }

            const auto value = instruction.get<TField>();
            if (IsZeroableField<Specification, TField> && value == 0) {
                decoded_instruction.operands.push_back(Operand::get<Operand::Type::Immediate>(0));
            }
            else {
                decoded_instruction.operands.push_back(
                    Operand::get<TField::operand_type>(value, access));
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

    if ((decoded_instruction.behaviors & Operand::Behavior::Absolute) == Operand::Behavior::None) {
        for (auto& operand : decoded_instruction.operands) {
            if (auto* branch_destination = std::get_if<Operand::BranchDestination>(
                    &operand.value)) {
                branch_destination->target += address;
            }
        }
    }

    return decoded_instruction;
}

template <typename TSpecification, typename TField>
[[nodiscard]] consteval Operand::Access
Decoder::get_access_type() {
    if constexpr (!is_register(TField::operand_type)) {
        return Operand::Access::None;
    }
    else {
        static_assert(HasAccesses<TSpecification>,
            "Layout has register fields but the specification doesn't provide an "
            "Accesses list");

        for (auto field_access : std::meta::template_arguments_of(
                 std::meta::dealias(^^typename TSpecification::Accesses))) {
            auto arguments = std::meta::template_arguments_of(std::meta::dealias(field_access));
            if (std::meta::dealias(arguments[0] /*field*/) == std::meta::dealias(^^TField)) {
                return std::meta::extract<Operand::Access>(arguments[1] /*access*/);
            }
        }

        return Operand::Access::None;
    }
}

} // namespace Revo

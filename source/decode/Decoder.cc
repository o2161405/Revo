#include "Decoder.hh"

#include "ppc/Concepts.hh"
#include "ppc/Layout.hh"
#include "util/Util.hh"

#include <algorithm>
#include <meta>

namespace Revo {

using namespace Revo::PPC;

namespace {

template <typename TSpecification, typename TField>
[[nodiscard]] consteval Operand::Access
operand_access() {
    if constexpr (!IsRegisterField<TField>) {
        return Operand::Access::None;
    }
    else {
        static_assert(HasAccesses<TSpecification>,
            "Layout has register fields but the specification doesn't provide accesses");

        for (const auto& access : TSpecification::accesses) {
            if (std::meta::dealias(access.field) == std::meta::dealias(^^TField)) {
                return access.access;
            }
        }

        return Operand::Access::None;
    }
}

} // namespace

std::expected<Decoder::Result, std::string>
Decoder::decode(std::span<const ELF::Function> functions) {
    Decoder::Result result;

    for (const auto& function : functions) {
        std::vector<Decode::Instruction> instructions;
        instructions.reserve(function.instructions.size());

        for (auto [index, raw] : Util::enumerate<u32>(function.instructions)) {
            const auto address = function.offset + index * INSTRUCTION_SIZE;

            auto decoded = parse(Instruction{raw}, address);
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

    Console::success("Decoded {} functions", result.functions.size());
    return result;
}

std::expected<Decode::Instruction, std::string>
Decoder::parse(Instruction instruction, u32 address) {
    const auto opcd = instruction.get<Layout::OPCD>();
    std::optional<u32> xo;

    template for (constexpr auto enumerator :
        std::define_static_array(std::meta::enumerators_of(^^Mnemonic))) {
        constexpr auto mnemonic = [:enumerator:];
        using Specification = InstructionSpecification<mnemonic>;

        if (opcd != Specification::opcd) {
            continue;
        }

        if (!instruction.valid<Specification>()) {
            continue;
        }

        using Layout = [:Specification::layout:];
        if constexpr (Layout::has_extended_opcode) {
            static_assert(HasExtendedOpcode<Specification>,
                "Layout has an extended opcode but the instruction specification doesn't "
                "provide the required fields");

            xo = instruction.extended_opcode<Layout>();
            if (xo != Specification::xo) {
                continue;
            }
        }

        if (instruction.uses_reserved_bits<Layout>()) {
            return std::unexpected(
                std::format("reserved bits set ({:#010x}) at {:#x}", instruction.raw(), address));
        }

        return make_instruction<mnemonic>(instruction, address);
    }

    if (xo) {
        return std::unexpected(
            std::format("unimplemented opcode ({}, xo {}) at {:#x}", opcd, *xo, address));
    }

    return std::unexpected(std::format("unimplemented opcode ({}) at {:#x}", opcd, address));
}

template <Mnemonic TMnemonic>
[[nodiscard]] constexpr Decode::Instruction
Decoder::make_instruction(Instruction instruction, u32 address) {
    using Specification = InstructionSpecification<TMnemonic>;

    static constexpr auto fields = std::define_static_array(
        std::meta::template_arguments_of(std::meta::dealias(Specification::layout)));

    if constexpr (HasZeroableField<Specification>) {
        static_assert(
            std::ranges::contains(fields, std::meta::dealias(Specification::zeroable_field)),
            "Zeroable_field references a field that isn't in the instruction's layout");
    }

    if constexpr (HasAccesses<Specification>) {
        template for (constexpr auto entry : Specification::accesses) {
            static_assert(std::ranges::contains(fields, std::meta::dealias(entry.field)),
                "Accesses references a field that isn't in the instruction's layout");
        }
    }

    Decode::Instruction decoded_instruction{.mnemonic = TMnemonic, .address = address};

    template for (constexpr auto field : fields) {
        using TField = [:field:];

        if constexpr (IsOperandField<TField>) {
            constexpr auto access = operand_access<Specification, TField>();

            if constexpr (IsRegisterField<TField>) {
                static_assert(access != Operand::Access::None,
                    "A register field is missing from the specification's accesses list");
            }

            const auto value = instruction.get<TField>();
            if (IsZeroableField<Specification, TField> && value == 0) {
                decoded_instruction.operands.push_back(Operand{.value = Operand::Immediate{0}});
            }
            else {
                decoded_instruction.operands.push_back(TField::make(value, access));
            }
        }
        else if constexpr (IsBehaviorField<TField>) {
            if constexpr (HasImpliedBehaviors<Specification>) {
                static_assert((TField::behavior & Specification::implied_behaviors) ==
                        Operand::Behavior::None,
                    "Operand and implied behaviour share one or more flags");
            }

            if (instruction.get<TField>() != 0) {
                decoded_instruction.behaviors |= TField::behavior;
            }
        }
    }

    if constexpr (HasImpliedBehaviors<Specification>) {
        decoded_instruction.behaviors |= Specification::implied_behaviors;
    }

    if constexpr (HasIndirectBranchSource<Specification>) {
        decoded_instruction.indirect_branch_source = Specification::indirect_branch_source;
    }

    if ((decoded_instruction.behaviors & Operand::Behavior::Absolute) == Operand::Behavior::None) {
        for (auto& operand : decoded_instruction.operands) {
            if (auto* ptr = std::get_if<Operand::BranchDestination>(&operand.value)) {
                ptr->address += address;
            }
        }
    }

    return decoded_instruction;
}

} // namespace Revo

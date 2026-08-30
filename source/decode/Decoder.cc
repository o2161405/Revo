#include "Decoder.hh"

#include "ppc/Concepts.hh"
#include "ppc/Form.hh"
#include "ppc/InstructionSpecification.hh"
#include "util/Util.hh"

#include <algorithm>
#include <meta>

// todo: find better names for parse and make_instruction because they are awful

namespace Revo::Decode {

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

template <typename TSpecification>
[[nodiscard]] constexpr bool
matches_constants(u32 raw) {
    if constexpr (HasConstants<TSpecification>) {
        template for (constexpr auto constant : TSpecification::constants) {
            using TField = [:constant.field:];
            if (TField::get(raw) != constant.value) {
                return false;
            }
        }
    }

    return true;
}

} // namespace

std::expected<std::vector<Function>, std::string>
decode(std::span<const ELF::Function> functions) {
    std::vector<Function> result;
    result.reserve(functions.size());

    for (const auto& function : functions) {
        auto decoded = Impl::decode_function(function);
        if (!decoded) {
            return std::unexpected(decoded.error());
        }

        result.push_back(std::move(*decoded));
    }

    Console::success("Decoded {} functions", result.size());
    return result;
}

namespace Impl {

std::expected<Function, std::string>
decode_function(const ELF::Function& function) {
    std::vector<Instruction> instructions;
    instructions.reserve(function.instructions.size());

    for (auto [index, raw] : Util::enumerate<u32>(function.instructions)) {
        const auto address = function.offset + index * INSTRUCTION_SIZE;

        auto decoded = parse(raw, address);
        if (!decoded) {
            return std::unexpected(decoded.error());
        }

        instructions.push_back(*decoded);
    }

    return Function{//
        .instructions = std::move(instructions),
        .relocations = function.relocations,
        .offset = function.offset,
        .size = function.size};
}

std::expected<Instruction, std::string>
parse(u32 raw, u32 address) {
    const auto opcd = Form::OPCD::get(raw);

    template for (constexpr auto enumerator :
        std::define_static_array(std::meta::enumerators_of(^^Mnemonic))) {
        constexpr auto mnemonic = [:enumerator:];
        using Specification = InstructionSpecification<mnemonic>;

        if (opcd != Specification::opcd) {
            continue;
        }

        if (!matches_constants<Specification>(raw)) {
            continue;
        }

        using TLayout = [:Specification::layout:];
        if constexpr (TLayout::has_extended_opcode) {
            static_assert(HasExtendedOpcode<Specification>,
                "Layout has an extended opcode but the instruction specification doesn't "
                "provide the required fields");

            if (TLayout::extended_opcode(raw) != Specification::xo) {
                continue;
            }
        }

        if (TLayout::uses_reserved_bits(raw)) {
            return std::unexpected(std::format( //
                "reserved bits set ({:#010x}) at {:#x}", raw, address));
        }

        return make_instruction<mnemonic>(raw, address);
    }

    return std::unexpected(std::format("unimplemented instruction at {:#x}", address));
}

template <PPC::Mnemonic TMnemonic>
constexpr Instruction
make_instruction(u32 raw, u32 address) {
    using TSpecification = InstructionSpecification<TMnemonic>;
    using TLayout = [:TSpecification::layout:];

    if constexpr (HasZeroableField<TSpecification>) {
        static_assert(std::ranges::contains(
                          TLayout::fields, std::meta::dealias(TSpecification::zeroable_field)),
            "Zeroable_field references a field that isn't in the instruction's layout");
    }

    if constexpr (HasAccesses<TSpecification>) {
        template for (constexpr auto entry : TSpecification::accesses) {
            static_assert(std::ranges::contains(TLayout::fields, std::meta::dealias(entry.field)),
                "Accesses references a field that isn't in the instruction's layout");
        }
    }

    Decode::Instruction decoded_instruction{.mnemonic = TMnemonic, .address = address};

    template for (constexpr auto field : TLayout::fields) {
        using TField = [:field:];

        if constexpr (IsOperandField<TField>) {
            constexpr auto access = operand_access<TSpecification, TField>();

            if constexpr (IsRegisterField<TField>) {
                static_assert(access != Operand::Access::None,
                    "A register field is missing from the specification's accesses list");
            }

            const auto value = TField::get(raw);
            if (IsZeroableField<TSpecification, TField> && value == 0) {
                decoded_instruction.operands.push_back(Operand{.value = Operand::Immediate{0}});
            }
            else {
                decoded_instruction.operands.push_back(TField::make(value, access));
            }
        }
        else if constexpr (IsBehaviorField<TField>) {
            if constexpr (HasImpliedBehaviors<TSpecification>) {
                static_assert((TField::behavior & TSpecification::implied_behaviors) ==
                        Operand::Behavior::None,
                    "Operand and implied behaviour share one or more flags");
            }

            if (TField::get(raw) != 0) {
                decoded_instruction.behaviors |= TField::behavior;
            }
        }
    }

    if constexpr (HasImpliedBehaviors<TSpecification>) {
        decoded_instruction.behaviors |= TSpecification::implied_behaviors;
    }

    if constexpr (HasIndirectBranchSource<TSpecification>) {
        decoded_instruction.indirect_branch_source = TSpecification::indirect_branch_source;
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

} // namespace Impl

} // namespace Revo::Decode

#pragma once

#include "elf/Types.hh"
#include "ppc/InstructionSpecification.hh"
#include "ppc/Mnemonic.hh"
#include "ppc/Operand.hh"
#include "util/Types.hh"

#include <flat_map>
#include <inplace_vector>
#include <meta>
#include <optional>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

namespace Revo::Decode {

using RelativeOffset = u32;

struct Instruction {
    static constexpr auto MAX_OPERANDS{5uz};

    [[nodiscard]] constexpr bool
    is_call() const {
        return (behaviors & PPC::Operand::Behavior::Link) != PPC::Operand::Behavior::None;
    }

    [[nodiscard]] constexpr bool
    conditional_branch() const {
        constexpr u8 ALWAYS{0b10100};

        for (const auto& operand : operands) {
            if (const auto* ptr = std::get_if<PPC::Operand::BranchOptions>(&operand.value)) {
                return (ptr->value & ALWAYS) != ALWAYS;
            }
        }

        return false;
    }

    [[nodiscard]] constexpr std::optional<u32>
    branch_destination() const {
        for (const auto& operand : operands) {
            if (const auto* ptr = std::get_if<PPC::Operand::BranchDestination>(&operand.value)) {
                return ptr->address;
            }
        }

        return std::nullopt;
    }

    // setting the return type to auto crashes gcc, it's fixed in trunk version
    // https://godbolt.org/z/rcEehdEW8
    template <auto TAmount>
    [[nodiscard]] constexpr std::array<PPC::Operand, TAmount>
    get_operands() const pre(operands.size() >= TAmount) {
        static_assert(TAmount <= MAX_OPERANDS,
            "Invalid number of operands, make sure you're excluding non-operand fields from your "
            "operand amount");
        std::array<PPC::Operand, TAmount> result{};
        std::ranges::copy_n(operands.begin(), TAmount, result.begin());
        return result;
    }

    PPC::Mnemonic mnemonic;
    u32 address;
    std::inplace_vector<PPC::Operand, MAX_OPERANDS> operands{};
    PPC::Operand::Behavior behaviors{};
    std::optional<PPC::Register::SPR> indirect_branch_source{};
};

struct Function {
    std::vector<Instruction> instructions;
    std::flat_map<RelativeOffset, std::vector<ELF::Rela>> relocations;
    u32 offset;
    u32 size;
};

} // namespace Revo::Decode

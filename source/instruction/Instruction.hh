#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>

#include "instruction/Mnemonic.hh"
#include "instruction/Operand.hh"

namespace Revo {

class Instruction {
public:
    static constexpr u32 INSTRUCTION_WIDTH = 32;
    static constexpr u32 INSTRUCTION_WIDTH_INDEX = INSTRUCTION_WIDTH - 1;

    template <u8 TStartIndex, u8 TEndIndex, typename TDataType,
        Operand::Type TOperandType = Operand::Type::None,
        Operand::Behavior TOperandBehavior = Operand::Behavior::None>
    struct Field {
        using data_type = TDataType;
        static constexpr Operand::Type operand_type = TOperandType;
        static constexpr Operand::Behavior operand_behaviour = TOperandBehavior;

        static_assert(TStartIndex <= TEndIndex, //
            "Start index must be less than or equal to the end index");
        static_assert(TEndIndex <= INSTRUCTION_WIDTH_INDEX,
            "End index must be less than or equal to the instruction width");
        static constexpr u8 bits = TEndIndex - TStartIndex + 1;
        static constexpr u8 shift = static_cast<u8>(INSTRUCTION_WIDTH_INDEX - TEndIndex);
        static constexpr u32 mask = (1ULL << bits) - 1;
    };

    template <u8 TStartIndex, u8 TEndIndex>
    struct ExtendedOpcode : Field<TStartIndex, TEndIndex, u16> {
        static constexpr bool is_extended_opcode = true;
    };

    template <typename... TFields>
    struct Layout {
        static constexpr u32 bits = (0 + ... + TFields::bits);
        static_assert(bits <= INSTRUCTION_WIDTH,
            "Instruction layout must be less than or equal to the instruction width");
    };

    constexpr explicit Instruction(u32 raw) : mRaw(raw) {}

    template <typename TField>
    [[nodiscard]] constexpr auto
    get() const {
        u32 raw_field = (mRaw >> TField::shift) & TField::mask;

        if constexpr (std::is_signed_v<typename TField::data_type>) {
            constexpr u32 shift = INSTRUCTION_WIDTH - TField::bits;
            return static_cast<typename TField::data_type>(
                static_cast<s32>(raw_field << shift) >> shift);
        }
        else {
            return static_cast<typename TField::data_type>(raw_field);
        }
    }

private:
    u32 mRaw;
};

struct DecodedInstruction {
    static constexpr auto MAX_OPERANDS{5uz};
    static constexpr auto BEHAVIOR_COUNT{4uz};

    constexpr void
    set_operands(std::initializer_list<Operand> operand_list) {
        for (auto [index, operand] : std::views::enumerate(operand_list)) {
            operands[index] = operand;
        }
    }

    [[nodiscard]] constexpr bool
    has_behavior(Operand::Behavior behavior) const {
        return behaviors[std::to_underlying(behavior) - 1];
    }

    Mnemonic mnemonic;
    std::array<Operand, MAX_OPERANDS> operands{};
    std::array<bool, BEHAVIOR_COUNT> behaviors{};
};

} // namespace Revo

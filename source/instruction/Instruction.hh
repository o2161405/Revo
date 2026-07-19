#pragma once

#include <concepts>
#include <inplace_vector>
#include <ranges>
#include <type_traits>

#include "instruction/Mnemonic.hh"
#include "instruction/Operand.hh"

namespace Revo {

class Instruction {
public:
    static constexpr u32 INSTRUCTION_WIDTH = 32;

    template <u8 TStartIndex, u8 TEndIndex, typename TDataType,
        Operand::Type TOperandType = Operand::Type::None,
        Operand::Behavior TOperandBehavior = Operand::Behavior::None>
    struct Field {
        using data_type = TDataType;
        static constexpr Operand::Type operand_type = TOperandType;
        static constexpr Operand::Behavior operand_behaviour = TOperandBehavior;

        static_assert(TStartIndex <= TEndIndex, //
            "Start index must be less than or equal to the end index");
        static_assert(TEndIndex < INSTRUCTION_WIDTH,
            "End index must be less than or equal to the instruction width");
        static constexpr u8 bits = TEndIndex - TStartIndex + 1;
        static constexpr u8 shift = static_cast<u8>(INSTRUCTION_WIDTH - TEndIndex - 1);
        static constexpr u32 mask = (1ULL << bits) - 1;
        static constexpr bool is_extended_opcode = false;
    };

    template <u8 TStartIndex, u8 TEndIndex>
    struct ExtendedOpcode : Field<TStartIndex, TEndIndex, u16> {
        static constexpr bool is_extended_opcode = true;
    };

    template <typename... TFields>
    struct Layout {
        static constexpr u32 bits = (0uz + ... + TFields::bits);
        static constexpr u32 mask = (0u | ... | (TFields::mask << TFields::shift));
        static constexpr bool has_extended_opcode = (false || ... || TFields::is_extended_opcode);
        static_assert(bits <= INSTRUCTION_WIDTH,
            "Instruction layout must be less than or equal to the instruction width");
        static_assert(std::popcount(mask) == bits,
            std::format("Instruction layout defines {} total bits are used, but due to one or more "
                        "incorrect shifts, {} total bits are used",
                bits, mask));

        [[nodiscard]] static constexpr bool
        uses_reserved_bits(u32 raw) {
            return (raw & ~mask) != 0;
        }

        /* clang-format off */
        [[nodiscard]] static constexpr u16
        extended_opcode(u32 raw) {
            return (0u | ... | (TFields::is_extended_opcode ?
                        static_cast<u16>((raw >> TFields::shift) & TFields::mask) : 0u));
        }
        /* clang-format on */
    };

    constexpr explicit Instruction(u32 raw) : mRaw(raw) {}

    [[nodiscard]] constexpr u32
    raw() const {
        return mRaw;
    }

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

    Mnemonic mnemonic;
    std::inplace_vector<Operand, MAX_OPERANDS> operands{};
    Operand::Behavior behaviors{};
};

} // namespace Revo

#pragma once

#include <concepts>
#include <type_traits>

namespace Revo::Architecture {

class Instruction {
public:
    static constexpr u32 INSTRUCTION_WIDTH = 32;
    static constexpr u32 INSTRUCTION_WIDTH_INDEX = INSTRUCTION_WIDTH - 1;

    template <u8 TStartIndex, u8 TEndIndex, typename TType>
    struct Field {
        using type = TType;

        STATIC_ASSERT(TStartIndex <= TEndIndex, //
            "Start index must be less than or equal to the end index");
        STATIC_ASSERT(TEndIndex <= INSTRUCTION_WIDTH_INDEX,
            "End index must be less than or equal to the instruction width");
        static constexpr u8 bits = TEndIndex - TStartIndex + 1;
        static constexpr u8 shift = static_cast<u8>(INSTRUCTION_WIDTH_INDEX - TEndIndex);
        static constexpr u32 mask = (1ULL << bits) - 1;
    };

    template <typename... TFields>
    struct Layout {
        static constexpr u32 bits = (0 + ... + TFields::bits);
        STATIC_ASSERT(bits <= INSTRUCTION_WIDTH,
            "Instruction layout must be less than or equal to the instruction width");
    };

    template <typename TField>
    [[nodiscard]] constexpr auto
    get() const {
        u32 raw_field = (mRaw >> TField::shift) & TField::mask;

        if constexpr (std::is_signed_v<typename TField::type>) {
            constexpr u32 shift = INSTRUCTION_WIDTH - TField::bits;
            return static_cast<typename TField::type>(
                static_cast<s32>(raw_field << shift) >> shift);
        } else {
            return static_cast<typename TField::type>(raw_field);
        }
    }

private:
    u32 mRaw;
};

} // namespace Revo::Architecture

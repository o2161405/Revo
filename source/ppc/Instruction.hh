#pragma once

#include "ppc/Common.hh"
#include "ppc/Concepts.hh"
#include "ppc/Field.hh"
#include "ppc/Mnemonic.hh"

#include <meta>
#include <ranges>
#include <type_traits>

namespace Revo::PPC {

class Instruction {
public:
    template <typename... TFields>
    struct Layout {
        static constexpr u32 bits = (0u + ... + TFields::bits);
        static constexpr u32 mask = (0u | ... | (TFields::mask << TFields::shift));
        static constexpr bool has_extended_opcode = (false || ... ||
            IsExtendedOpcodeField<TFields>);

        static_assert(bits <= INSTRUCTION_WIDTH,
            "Instruction layout must be less than or equal to the instruction width");
        static_assert(std::popcount(mask) == bits,
            std::format("Instruction layout defines {} total bits are used, but due to one or more "
                        "incorrect shifts, {} total bits are used",
                bits, mask));
    };

    constexpr explicit Instruction(u32 raw) : mRaw(raw) {}

    [[nodiscard]] constexpr u32
    raw() const {
        return mRaw;
    }

    template <typename TLayout>
    [[nodiscard]] constexpr bool
    uses_reserved_bits() const {
        return (mRaw & ~TLayout::mask) != 0;
    }

    template <typename TLayout>
    [[nodiscard]] constexpr auto
    extended_opcode() const
        requires TLayout::has_extended_opcode
    {
        static constexpr auto fields = std::define_static_array(
            std::meta::template_arguments_of(std::meta::dealias(^^TLayout)));

        template for (constexpr auto field : fields) {
            using TField = [:field:];
            if constexpr (IsExtendedOpcodeField<TField>) {
                return (mRaw >> TField::shift) & TField::mask;
            }
        }
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

    template <typename TSpecification>
    [[nodiscard]] constexpr bool
    valid() const {
        if constexpr (HasConstants<TSpecification>) {
            template for (constexpr auto constant : TSpecification::constants) {
                using TField = [:constant.field:];
                if (get<TField>() != constant.value) {
                    return false;
                }
            }
        }

        return true;
    }

private:
    u32 mRaw;
};

} // namespace Revo::PPC

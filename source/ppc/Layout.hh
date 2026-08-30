#pragma once

#include "ppc/Common.hh"
#include "ppc/Concepts.hh"

#include <array>
#include <bit>
#include <format>
#include <meta>
#include <utility>

namespace Revo::PPC {

template <typename... TFields>
struct Layout {
    static constexpr u32 bits = (0u + ... + TFields::bits);
    static constexpr u32 mask = (0u | ... | (TFields::mask << TFields::shift));
    static constexpr std::array fields{^^TFields...};
    static constexpr bool has_extended_opcode = (false || ... || IsExtendedOpcodeField<TFields>);

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

    [[nodiscard]] static constexpr u16
    extended_opcode(u32 raw)
        requires has_extended_opcode
    {
        template for (constexpr auto field : fields) {
            using TField = [:field:];
            if constexpr (IsExtendedOpcodeField<TField>) {
                return TField::get(raw);
            }
        }

        std::unreachable();
    }
};

} // namespace Revo::PPC

#pragma once

#include "elf/Types.hh"

namespace Revo::ELF {

#pragma pack(push, 1)
struct SymbolHeader {
    /// \cond
    u32 st_name;
    u32 st_value;
    u32 st_size;
    u8 st_info;
    u8 st_other;
    u16 st_shndx;
    /// \endcond
};
#pragma pack(pop)

struct Symbol {
    SymbolHeader header{};
    std::string name;

    [[nodiscard]] constexpr u8
    type() const {
        return header.st_info & 0xF;
    }

    [[nodiscard]] constexpr bool
    contains(u32 address) const {
        return address >= header.st_value && address < (header.st_value + header.st_size);
    }
};

} // namespace Revo::ELF

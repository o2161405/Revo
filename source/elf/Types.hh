#pragma once

#include <array>

namespace Revo::ELF {

using RelativeOffset = u32;

#pragma pack(push, 1)
struct Rela {
    /// \cond
    u32 r_offset;
    u32 r_info;
    s32 r_addend;
    /// \endcond

    [[nodiscard]] constexpr u32
    symbol_index() const {
        return r_info >> 8;
    }

    [[nodiscard]] constexpr u8
    type() const {
        return r_info & 0xFF;
    }
};

struct Symbol {
    /// \cond
    u32 st_name;
    u32 st_value;
    u32 st_size;
    u8 st_info;
    u8 st_other;
    u16 st_shndx;
    /// \endcond

    [[nodiscard]] constexpr u8
    type() const {
        return st_info & 0xF;
    }

    [[nodiscard]] constexpr u8
    bind() const {
        return st_info >> 4;
    }

    [[nodiscard]] constexpr bool
    contains(u32 address) const {
        return address >= st_value && address < (st_value + st_size);
    }
};

struct ELFHeader {
    static constexpr auto IDENT_COUNT{16uz};

    /// \cond
    std::array<u8, IDENT_COUNT> e_ident;
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    u32 e_entry;
    u32 e_phoff;
    u32 e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
    /// \endcond
};

struct SectionHeader {
    /// \cond
    u32 sh_name;
    u32 sh_type;
    u32 sh_flags;
    u32 sh_addr;
    u32 sh_offset;
    u32 sh_size;
    u32 sh_link;
    u32 sh_info;
    u32 sh_addralign;
    u32 sh_entsize;
    /// \endcond

    [[nodiscard]] constexpr bool
    contains(const Symbol& symbol) const {
        if (symbol.st_value < sh_addr) {
            return false;
        }

        const RelativeOffset offset = symbol.st_value - sh_addr;
        if (offset >= sh_size || symbol.st_size > sh_size - offset) {
            return false;
        }

        return true;
    }
};
#pragma pack(pop)

} // namespace Revo::ELF

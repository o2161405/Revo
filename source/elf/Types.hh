#pragma once

#include <array>
#include <flat_map>
#include <vector>

namespace Revo::ELF {

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
#pragma pack(pop)

struct Function {
    std::vector<u32> instructions;
    std::flat_map<u32, std::vector<Rela>> relocations;
    u32 offset;
    u32 size;

    [[nodiscard]] constexpr bool
    contains(u32 address) const {
        return address >= offset && address < (offset + size);
    }
};

} // namespace Revo::ELF

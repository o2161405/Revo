#pragma once

#include "elf/Types.hh"

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace Revo::ELF {

using SectionIndex = u16;

#pragma pack(push, 1)
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
};
#pragma pack(pop)

struct Section {
    std::vector<std::byte> data;
    SectionHeader header{};
    SectionIndex index{};
    std::string name;

    [[nodiscard]] std::optional<std::span<const std::byte>>
    bytes(u32 address, u32 size) const {
        if (address < header.sh_addr) {
            return std::nullopt;
        }

        const u32 offset = address - header.sh_addr;
        if (offset >= data.size() || size > data.size() - offset) {
            return std::nullopt;
        }

        return std::span{data}.subspan(offset, size);
    }
};

} // namespace Revo::ELF

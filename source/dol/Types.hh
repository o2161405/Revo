#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace Revo::DOL {

inline constexpr std::string_view WATERMARK{//
    "Protected by Revo. Learn more at github.com/o2161405/Revo"};

#pragma pack(push, 1)
struct DOLHeader {
    static constexpr auto MAX_TEXT_SECTIONS{7};
    static constexpr auto MAX_DATA_SECTIONS{11};

    std::array<u32, MAX_TEXT_SECTIONS> text_offsets{};
    std::array<u32, MAX_DATA_SECTIONS> data_offsets{};
    std::array<u32, MAX_TEXT_SECTIONS> text_loading_addresses{};
    std::array<u32, MAX_DATA_SECTIONS> data_loading_addresses{};
    std::array<u32, MAX_TEXT_SECTIONS> text_section_sizes{};
    std::array<u32, MAX_DATA_SECTIONS> data_section_sizes{};
    u32 bss_address{};
    u32 bss_size{};
    u32 entry_point{};
    std::array<u8, 0x1C> padding{};
};
#pragma pack(pop)

static_assert(sizeof(DOLHeader) == 0x100);

struct Section {
    static constexpr u32 ALIGNMENT{64};

    enum class Type : u8 {
        Text,
        Data,
        BSS,
    };

    std::vector<std::span<const std::byte>> data{};
    u32 address{};
    u32 size{};
    u32 offset{};
    Type type{};

    [[nodiscard]] constexpr u32
    end() const {
        return address + size;
    }

    [[nodiscard]] constexpr bool
    overlaps(const Section& other) const {
        return address < other.end() && other.address < end();
    }

    [[nodiscard]] constexpr bool
    is_text() const {
        return type == Type::Text;
    }

    [[nodiscard]] constexpr bool
    is_data() const {
        return type == Type::Data;
    }

    [[nodiscard]] constexpr bool
    is_bss() const {
        return type == Type::BSS;
    }
};

} // namespace Revo::DOL

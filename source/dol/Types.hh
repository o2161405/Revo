#pragma once

namespace Revo::DOL {

#pragma pack(push, 1)
struct DOLHeader {
    static constexpr auto TEXT_SECTIONS{7uz};
    static constexpr auto DATA_SECTIONS{11uz};

    std::array<u32, TEXT_SECTIONS> text_offsets{};
    std::array<u32, DATA_SECTIONS> data_offsets{};
    std::array<u32, TEXT_SECTIONS> text_loading_addresses{};
    std::array<u32, DATA_SECTIONS> data_loading_addresses{};
    std::array<u32, TEXT_SECTIONS> text_section_sizes{};
    std::array<u32, DATA_SECTIONS> data_section_sizes{};
    u32 bss_address{};
    u32 bss_size{};
    u32 entry_point{};
    std::array<u8, 0x1C> _{};
};
#pragma pack(pop)

} // namespace Revo::DOL

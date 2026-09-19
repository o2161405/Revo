#pragma once

#include "dol/Types.hh"
#include "elf/Object.hh"

#include <algorithm>
#include <expected>
#include <filesystem>
#include <string>

namespace Revo::DOL {

[[nodiscard]] std::expected<void, std::string>
write(const std::filesystem::path& path, const ELF::Object& object);

namespace Impl {

// Writing steps
[[nodiscard]] std::expected<void, std::string>
make_sections(std::vector<Section>& sections, std::span<const ELF::Section> elf_sections);

[[nodiscard]] std::expected<void, std::string>
check_overlaps(std::span<const Section> sections)
    pre(std::ranges::is_sorted(sections, {}, &Section::address));

void
merge_sections(std::vector<Section>& sections) //
    pre(std::ranges::is_sorted(sections, {}, &Section::address));

[[nodiscard]] std::expected<void, std::string>
place_sections(std::span<Section> sections) //
    pre(std::ranges::is_sorted(sections, {}, &Section::address));

void
make_header(DOLHeader& header, std::span<const Section> sections, u32 entry_point) //
    pre(std::ranges::count_if(sections, &Section::is_text) <= DOLHeader::MAX_TEXT_SECTIONS) //
    pre(std::ranges::count_if(sections, &Section::is_data) <= DOLHeader::MAX_DATA_SECTIONS);

[[nodiscard]] std::expected<void, std::string>
write_file(const std::filesystem::path& path, const DOLHeader& header, //
    std::span<const Section> sections);

} // namespace Impl

} // namespace Revo::DOL

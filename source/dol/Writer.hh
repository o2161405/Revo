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
make_sections(Output& output, const ELF::Object& object);

[[nodiscard]] std::expected<void, std::string>
check_overlaps(const Output& output)
    pre(std::ranges::is_sorted(output.sections, {}, &Section::address));

void
merge_sections(Output& output) //
    pre(std::ranges::is_sorted(output.sections, {}, &Section::address));

[[nodiscard]] std::expected<void, std::string>
place_sections(Output& output) //
    pre(std::ranges::is_sorted(output.sections, {}, &Section::address));

void
make_header(Output& output, const ELF::Object& object) //
    pre(std::ranges::count_if(output.sections, &Section::is_text) //
        <= DOLHeader::MAX_TEXT_SECTIONS) //
    pre(std::ranges::count_if(output.sections, &Section::is_data) //
        <= DOLHeader::MAX_DATA_SECTIONS);

[[nodiscard]] std::expected<void, std::string>
write_file(const Output& output, const std::filesystem::path& path);

} // namespace Impl

} // namespace Revo::DOL

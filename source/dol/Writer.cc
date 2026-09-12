#include "Writer.hh"

#include "util/Util.hh"

#include <format>
#include <fstream>

namespace Revo::DOL {

namespace {

[[nodiscard]] constexpr Section::Type
section_type(const ELF::Section& section) {
    if (section.is_nobits()) {
        return Section::Type::BSS;
    }

    if (section.is_executable()) {
        return Section::Type::Text;
    }

    return Section::Type::Data;
}

} // namespace

std::expected<void, std::string>
write(const std::filesystem::path& path, const ELF::Object& object) {
    DOLHeader header{};
    std::vector<Section> sections;

    return Impl::make_sections(sections, object.sections)
        .transform([&] { std::ranges::sort(sections, {}, &Section::address); })
        .and_then([&] { return Impl::check_overlaps(sections); })
        .transform([&] { Impl::merge_sections(sections); })
        .and_then([&] { return Impl::place_sections(sections); })
        .transform([&] { return Impl::make_header(header, sections, object.elf_header.e_entry); })
        .and_then([&] { return Impl::write_file(path, header, sections); });
}

namespace Impl {

std::expected<void, std::string>
make_sections(std::vector<Section>& sections, std::span<const ELF::Section> elf_sections) {
    const auto is_empty = [](const auto& section) { //
        return section.header.sh_size == 0;
    };

    for (const auto& section : elf_sections //
            | std::views::filter(&ELF::Section::is_allocated) //
            | std::views::filter(std::not_fn(is_empty))) {
        Section result{//
            .address = section.header.sh_addr,
            .size = section.header.sh_size,
            .type = section_type(section)};

        if (result.type != Section::Type::BSS) {
            contract_assert(section.data.size() == result.size);
            result.data.push_back(std::span{section.data});
        }

        sections.push_back(std::move(result));
    }

    return {};
}

std::expected<void, std::string>
check_overlaps(std::span<const Section> sections) {
    for (const auto& [previous, next] : std::views::pairwise(sections)) {
        if (previous.overlaps(next)) {
            return std::unexpected(std::format( //
                "section {:#x} (size of {:#x}) overlaps with section {:#x}", //
                previous.address, previous.size, next.address));
        }
    }

    return {};
}

void
merge_sections(std::vector<Section>& sections) {
    const auto can_merge = [](const auto& previous, const auto& next) {
        return previous.type == next.type && previous.end() == next.address;
    };

    std::vector<Section> result;
    result.reserve(sections.size());

    for (const auto& sections_chunk : sections | std::views::chunk_by(can_merge)) {
        auto& merged = result.emplace_back(sections_chunk.front());

        for (const auto& section : sections_chunk | std::views::drop(1)) {
            merged.size += section.size;
            merged.data.append_range(section.data);
        }
    }

    sections = std::move(result);
}

std::expected<void, std::string>
place_sections(std::span<Section> sections) {
    constexpr u32 SECTIONS_START = Util::align_up(
        sizeof(DOLHeader) + WATERMARK.size(), Section::ALIGNMENT);

    auto text_sections = sections | std::views::filter(&Section::is_text);
    auto data_sections = sections | std::views::filter(&Section::is_data);

    if (std::ranges::distance(text_sections) > DOLHeader::MAX_TEXT_SECTIONS) {
        return std::unexpected(std::format("{} text sections exceeds the maximum of {}",
            std::ranges::distance(text_sections), DOLHeader::MAX_TEXT_SECTIONS));
    }

    if (std::ranges::distance(data_sections) > DOLHeader::MAX_DATA_SECTIONS) {
        return std::unexpected(std::format("{} data sections exceeds the maximum of {}",
            std::ranges::distance(data_sections), DOLHeader::MAX_DATA_SECTIONS));
    }

    auto offset = SECTIONS_START;
    for (auto& section : std::views::concat(text_sections, data_sections)) {
        section.offset = offset;
        offset += Util::align_up(section.size, Section::ALIGNMENT);
    }

    return {};
}

// todo: should this take header by reference or return a DOLHeader? the latter
// is better but it's a little out of place with the void functions around it
void
make_header(DOLHeader& header, std::span<const Section> sections, u32 entry_point) {
    header.entry_point = entry_point;

    auto text_sections = sections | std::views::filter(&Section::is_text);
    auto data_sections = sections | std::views::filter(&Section::is_data);

    std::ranges::copy(text_sections | std::views::transform(&Section::offset), //
        header.text_offsets.begin());
    std::ranges::copy(text_sections | std::views::transform(&Section::address), //
        header.text_loading_addresses.begin());
    std::ranges::copy(text_sections | std::views::transform(&Section::size), //
        header.text_section_sizes.begin());

    std::ranges::copy(data_sections | std::views::transform(&Section::offset), //
        header.data_offsets.begin());
    std::ranges::copy(data_sections | std::views::transform(&Section::address), //
        header.data_loading_addresses.begin());
    std::ranges::copy(data_sections | std::views::transform(&Section::size), //
        header.data_section_sizes.begin());

    if (const auto bss = std::ranges::find_if(sections, &Section::is_bss); bss != sections.end()) {
        header.bss_address = bss->address;
        header.bss_size = bss->size;
    }
}

std::expected<void, std::string>
write_file(const std::filesystem::path& path, const DOLHeader& header, //
    std::span<const Section> sections) //
{
    std::ofstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return std::unexpected("failed to open file");
    }

    auto swapped = header;
    Util::byteswap(swapped);

    if (!stream.write(reinterpret_cast<const char*>(&swapped), sizeof(DOLHeader))) {
        return std::unexpected("failed to write header.");
    }

    if (!stream.write(WATERMARK.data(), WATERMARK.size())) {
        return std::unexpected("failed to write watermark");
    }

    for (const auto& section : sections | std::views::filter(std::not_fn(&Section::is_bss))) {
        stream.seekp(section.offset);

        for (const auto bytes : section.data) {
            if (!stream.write(reinterpret_cast<const char*>(bytes.data()), bytes.size())) {
                return std::unexpected(std::format( //
                    "failed to write section {:#x}", section.address));
            }
        }
    }

    return {};
}

} // namespace Impl

} // namespace Revo::DOL

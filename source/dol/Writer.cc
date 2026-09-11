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
    Output output;

    return Impl::make_sections(output, object)
        .transform([&] { std::ranges::sort(output.sections, {}, &Section::address); })
        .and_then([&] { return Impl::check_overlaps(output); })
        .transform([&] { Impl::merge_sections(output); })
        .and_then([&] { return Impl::place_sections(output); })
        .transform([&] { Impl::make_header(output, object); })
        .and_then([&] { return Impl::write_file(output, path); });
}

namespace Impl {

std::expected<void, std::string>
make_sections(Output& output, const ELF::Object& object) {
    const auto is_empty = [](const ELF::Section& section) { //
        return section.header.sh_size == 0;
    };

    for (const auto& section : object.sections //
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

        output.sections.push_back(std::move(result));
    }

    return {};
}

std::expected<void, std::string>
check_overlaps(const Output& output) {
    for (const auto& [previous, next] : std::views::pairwise(output.sections)) {
        if (previous.overlaps(next)) {
            return std::unexpected(std::format( //
                "section at {:#x} (size {:#x}) overlaps with section at {:#x}.", //
                previous.address, previous.size, next.address));
        }
    }

    return {};
}

void
merge_sections(Output& output) {
    const auto can_merge = [](const Section& previous, const Section& next) {
        return previous.type == next.type && previous.end() == next.address;
    };

    std::vector<Section> result;
    result.reserve(output.sections.size());

    for (const auto& sections : output.sections | std::views::chunk_by(can_merge)) {
        auto& merged = result.emplace_back(sections.front());

        for (const auto& section : sections | std::views::drop(1)) {
            merged.size += section.size;
            merged.data.append_range(section.data);
        }
    }

    output.sections = std::move(result);
}

std::expected<void, std::string>
place_sections(Output& output) {
    constexpr u32 SECTIONS_START = Util::align_up(
        sizeof(DOLHeader) + WATERMARK.size(), Section::ALIGNMENT);

    auto text_sections = output.sections | std::views::filter(&Section::is_text);
    auto data_sections = output.sections | std::views::filter(&Section::is_data);

    if (std::ranges::distance(text_sections) > DOLHeader::MAX_TEXT_SECTIONS) {
        return std::unexpected(std::format("got {} text sections (expected <={}).",
            std::ranges::distance(text_sections), DOLHeader::MAX_TEXT_SECTIONS));
    }

    if (std::ranges::distance(data_sections) > DOLHeader::MAX_DATA_SECTIONS) {
        return std::unexpected(std::format("got {} data sections (expected <={}).",
            std::ranges::distance(data_sections), DOLHeader::MAX_DATA_SECTIONS));
    }

    u32 offset = SECTIONS_START;
    for (auto& section : std::views::concat(text_sections, data_sections)) {
        section.offset = offset;
        offset += Util::align_up(section.size, Section::ALIGNMENT);
    }

    return {};
}

void
make_header(Output& output, const ELF::Object& object) {
    auto& header = output.header;
    header.entry_point = object.elf_header.e_entry;

    auto text_sections = output.sections | std::views::filter(&Section::is_text);
    auto data_sections = output.sections | std::views::filter(&Section::is_data);

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

    if (const auto bss = std::ranges::find_if(output.sections, &Section::is_bss);
        bss != output.sections.end()) {
        header.bss_address = bss->address;
        header.bss_size = bss->size;
    }
}

std::expected<void, std::string>
write_file(const Output& output, const std::filesystem::path& path) {
    std::ofstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return std::unexpected("failed to open file.");
    }

    auto header = output.header;
    Util::byteswap(header);

    if (!stream.write(reinterpret_cast<const char*>(&header), sizeof(header))) {
        return std::unexpected("failed to write header.");
    }

    if (!stream.write(WATERMARK.data(), WATERMARK.size())) {
        return std::unexpected("failed to write watermark.");
    }

    for (const auto& section : output.sections //
            | std::views::filter(std::not_fn(&Section::is_bss))) {
        stream.seekp(section.offset);

        for (const auto bytes : section.data) {
            if (!stream.write(reinterpret_cast<const char*>(bytes.data()), bytes.size())) {
                return std::unexpected(std::format( //
                    "failed to write section at {:#x}.", section.address));
            }
        }
    }

    return {};
}

} // namespace Impl

} // namespace Revo::DOL

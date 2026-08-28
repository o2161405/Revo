#include "Parser.hh"

#include "ppc/Common.hh"
#include "util/Config.hh"
#include "util/Util.hh"

#include <algorithm>
#include <cstring>
#include <format>
#include <functional>
#include <memory>
#include <ranges>
#include <type_traits>

namespace Revo::ELF {

std::optional<const Section&>
Parser::Result::get_section(std::string_view name) const {
    if (const auto it = std::ranges::find(sections, name, &Section::name); it != sections.end()) {
        return *it;
    }

    return std::nullopt;
}

std::expected<Parser::Result, std::string>
Parser::parse(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return std::unexpected("failed to open file");
    }

    return parse(stream);
}

std::expected<Parser::Result, std::string>
Parser::parse(std::ifstream& stream) {
    Parser parser(stream);

    return parser.read_elf_header()
        .and_then(std::bind_front(&Parser::read_sections, &parser))
        .and_then(std::bind_front(&Parser::read_section_names, &parser))
        .and_then(std::bind_front(&Parser::read_symbol_table, &parser))
        .and_then(std::bind_front(&Parser::read_revo_functions, &parser))
        .and_then(std::bind_front(&Parser::read_revo_relocations, &parser))
        .and_then(std::bind_front(&Parser::check_functions, &parser))
        .and_then(std::bind_front(&Parser::check_relocations, &parser))
        .transform([&]() {
            Console::success("Parsed {} functions and {} relocations",
                parser.mResult.revo_functions.size(), parser.mResult.revo_relocations.size());
            return std::move(parser.mResult);
        });
}

std::expected<void, std::string>
Parser::read_elf_header() {
    constexpr auto ELF_MAGIC = std::to_array<u8>({0x7F, 'E', 'L', 'F'});
    constexpr auto EM_PPC{20uz};
    constexpr auto EI_CLASS{4uz};
    constexpr auto ELFCLASS32{1uz};

    if (!mStream.read(reinterpret_cast<char*>(&mResult.elf_header), sizeof(mResult.elf_header))) {
        return std::unexpected("reached EOF whilst interpreting header");
    }

    if (!std::ranges::starts_with(mResult.elf_header.e_ident, ELF_MAGIC)) {
        return std::unexpected(std::format( //
            "got ELF magic of 0x{:02X}{:02X}{:02X}{:02X} (expected 0x7F454C46)",
            mResult.elf_header.e_ident[0], mResult.elf_header.e_ident[1],
            mResult.elf_header.e_ident[2], mResult.elf_header.e_ident[3]));
    }

    Util::byteswap(mResult.elf_header);

    if (mResult.elf_header.e_machine != EM_PPC) {
        return std::unexpected(std::format(
            "got e_machine value of {} (expected {})", mResult.elf_header.e_machine, EM_PPC));
    }

    if (mResult.elf_header.e_ident[EI_CLASS] != ELFCLASS32) {
        return std::unexpected(std::format( //
            "got EI_CLASS value of {} (expected {}). Make sure your binary is 32-bit.",
            mResult.elf_header.e_ident[EI_CLASS], ELFCLASS32));
    }

    return {};
}

std::expected<void, std::string>
Parser::read_sections() {
    constexpr auto SHT_NOBITS{8uz};

    mResult.sections.resize(mResult.elf_header.e_shnum);

    mStream.seekg(mResult.elf_header.e_shoff);
    for (auto [index, section] : std::views::enumerate(mResult.sections)) {
        if (!mStream.read(reinterpret_cast<char*>(&section.header), sizeof(SectionHeader))) {
            return std::unexpected("reached EOF whilst reading section headers");
        }

        Util::byteswap(section.header);
        section.index = static_cast<SectionIndex>(index);
    }

    for (auto& section : mResult.sections) {
        if (section.header.sh_type == SHT_NOBITS) {
            continue;
        }

        section.data.resize(section.header.sh_size);
        mStream.seekg(section.header.sh_offset);
        if (!mStream.read(reinterpret_cast<char*>(section.data.data()), section.header.sh_size)) {
            return std::unexpected("reached EOF whilst reading section data");
        }
    }

    return {};
}

std::expected<void, std::string>
Parser::read_section_names() {
    constexpr auto SHN_UNDEF{0uz};
    constexpr auto SHT_STRTAB{3uz};

    if (mResult.elf_header.e_shstrndx == SHN_UNDEF) {
        return std::unexpected(std::format( //
            "got string table index of {} (expected non-zero value)", SHN_UNDEF));
    }

    if (mResult.elf_header.e_shstrndx >= mResult.sections.size()) {
        return std::unexpected(std::format( //
            "ELF header size ({}) and section header size ({}) do not match",
            mResult.elf_header.e_shstrndx, mResult.sections.size()));
    }

    const auto& strtab = mResult.sections[mResult.elf_header.e_shstrndx];
    if (strtab.header.sh_type != SHT_STRTAB) {
        return std::unexpected(std::format(
            "got SHT_STRTAB type flag of {} (expected {})", strtab.header.sh_type, SHT_STRTAB));
    }

    const std::string_view string_table{
        reinterpret_cast<const char*>(strtab.data.data()), strtab.data.size()};

    for (auto& section : mResult.sections) {
        const auto offset = section.header.sh_name;
        if (offset >= string_table.size()) {
            return std::unexpected(std::format( //
                "section {} has a name offset of {} (expected <{})", //
                section.index, offset, string_table.size()));
        }

        const auto end = string_table.find('\0', offset);
        section.name = string_table.substr(offset, end - offset);
    }

    return {};
}

std::expected<void, std::string>
Parser::read_symbol_table() {
    constexpr auto SHT_SYMTAB{2uz};

    const auto symtab_section = mResult.get_section(".symtab");
    if (!symtab_section) {
        return std::unexpected("failed to find section .symtab");
    }

    if (symtab_section->header.sh_type != SHT_SYMTAB) {
        return std::unexpected(std::format( //
            "got SHT_SYMTAB type flag of {} (expected {})", //
            symtab_section->header.sh_type, SHT_SYMTAB));
    }

    auto symbols = read_table<Symbol>(*symtab_section);
    if (!symbols) {
        return std::unexpected(symbols.error());
    }

    mResult.symbols = std::move(*symbols);
    return {};
}

std::expected<void, std::string>
Parser::read_revo_relocations() {
    const auto rela_section = mResult.get_section(Config::RelaInputSection);
    if (!rela_section) {
        Console::warning("No relocation section found, attempting parse anyway");
        return {};
    }

    auto relocations = read_table<Rela>(*rela_section);
    if (!relocations) {
        return std::unexpected(relocations.error());
    }

    mResult.revo_relocations = std::move(*relocations);
    return {};
}

std::expected<void, std::string>
Parser::read_revo_functions() {
    constexpr auto STT_FUNC{2uz};

    const auto input_section = mResult.get_section(Config::InputSection);
    if (!input_section) {
        return std::unexpected(std::format("failed to get section {}", Config::InputSection));
    }

    for (const auto& symbol : mResult.symbols) {
        if (symbol.st_shndx != input_section->index) {
            continue;
        }

        if (symbol.type() != STT_FUNC) {
            Console::debug("Skipped symbol {:#x}: type {} (expected {})", //
                symbol.st_value, symbol.type(), STT_FUNC);
            continue;
        }

        if (symbol.st_value % PPC::INSTRUCTION_SIZE != 0) {
            return std::unexpected(std::format( //
                "function {:#x} is not aligned to {} bytes", //
                symbol.st_value, PPC::INSTRUCTION_SIZE));
        }

        if (symbol.st_size == 0) {
            return std::unexpected(std::format("function {:#x} has zero size", symbol.st_value));
        }

        if (symbol.st_size % PPC::INSTRUCTION_SIZE != 0) {
            return std::unexpected(std::format( //
                "function {:#x} (size of {}) is not a multiple of {} bytes", //
                symbol.st_value, symbol.st_size, PPC::INSTRUCTION_SIZE));
        }

        const auto bytes = input_section->bytes(symbol.st_value, symbol.st_size);
        if (!bytes) {
            return std::unexpected(std::format(
                "function {:#x} isn't contained within the input section", symbol.st_value));
        }

        std::vector<u32> instructions(symbol.st_size / PPC::INSTRUCTION_SIZE);
        std::memcpy(instructions.data(), bytes->data(), bytes->size());
        Util::byteswap(instructions);

        std::flat_map<u32, std::vector<Rela>> relocations;

        for (const auto& relocation : mResult.revo_relocations) {
            if (!symbol.contains(relocation.r_offset)) {
                continue;
            }

            relocations[relocation.r_offset - symbol.st_value].push_back(relocation);
            Console::debug("Relocation {:#x} assigned to function {:#x}", //
                relocation.r_offset, symbol.st_value);
        }

        mResult.revo_functions.push_back({//
            .instructions = std::move(instructions),
            .relocations = std::move(relocations),
            .offset = symbol.st_value,
            .size = symbol.st_size});
    }

    return {};
}

std::expected<void, std::string>
Parser::check_functions() {
    std::ranges::sort(mResult.revo_functions, {}, &Function::offset);

    for (const auto& [previous, next] : std::views::pairwise(mResult.revo_functions)) {
        if (previous.offset + previous.size > next.offset) {
            return std::unexpected(std::format( //
                "function {:#x} (size of {:#x}) overlaps with function {:#x}", //
                previous.offset, previous.size, next.offset));
        }
    }

    return {};
}

std::expected<void, std::string>
Parser::check_relocations() const {
    for (const auto& relocation : mResult.revo_relocations) {
        const auto it = std::ranges::upper_bound(
            mResult.revo_functions, relocation.r_offset, {}, &Function::offset);

        if (it == mResult.revo_functions.begin() || !std::prev(it)->contains(relocation.r_offset)) {
            return std::unexpected(std::format(
                "relocation {:#x} isn't referenced by a function", relocation.r_offset));
        }
    }

    return {};
}

template <typename TType>
[[nodiscard]] std::expected<std::vector<TType>, std::string>
Parser::read_table(const Section& section) {
    if (section.data.size() % sizeof(TType) != 0) {
        return std::unexpected(std::format( //
            "section {} (size of {}) is not a multiple of {} bytes", //
            section.name, section.data.size(), sizeof(TType)));
    }

    if (section.data.empty()) {
        return std::vector<TType>{};
    }

    std::vector<TType> entries(section.data.size() / sizeof(TType));
    std::memcpy(entries.data(), section.data.data(), section.data.size());
    Util::byteswap(entries);

    return entries;
}

} // namespace Revo::ELF

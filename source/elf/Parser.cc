#include "Parser.hh"

#include "elf/Symbol.hh"
#include "ppc/Common.hh"
#include "util/Util.hh"

#include <algorithm>
#include <array>
#include <cstring>
#include <flat_map>
#include <format>
#include <fstream>
#include <ranges>

namespace Revo::ELF {

std::expected<Object, std::string>
parse(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return std::unexpected("failed to open file");
    }

    return parse(stream);
}

std::expected<Object, std::string>
parse(std::istream& stream) {
    Object object;

    return Impl::read_elf_header(object, stream)
        .and_then([&] { return Impl::read_sections(object, stream); })
        .and_then([&] { return Impl::read_section_names(object); })
        .and_then([&] { return Impl::read_symbols(object); })
        .and_then([&] { return Impl::read_revo_relocations(object); })
        .and_then([&] { return Impl::read_revo_functions(object); })
        .and_then([&] { return Impl::check_functions(object); })
        .and_then([&] { return Impl::check_relocations(object); })
        .transform([&] {
            Console::success("Parsed {} functions and {} relocations", //
                object.revo_functions.size(), object.revo_relocations.size());
            return std::move(object);
        });
}

namespace Impl {

std::expected<void, std::string>
read_elf_header(Object& object, std::istream& stream) {
    constexpr auto ELF_MAGIC = std::to_array<u8>({0x7F, 'E', 'L', 'F'});
    constexpr auto EM_PPC{20uz};
    constexpr auto EI_CLASS{4uz};
    constexpr auto ELFCLASS32{1uz};

    if (!stream.read(reinterpret_cast<char*>(&object.elf_header), sizeof(object.elf_header))) {
        return std::unexpected("reached EOF whilst interpreting header");
    }

    if (!std::ranges::starts_with(object.elf_header.e_ident, ELF_MAGIC)) {
        return std::unexpected(std::format( //
            "got ELF magic of 0x{:02X}{:02X}{:02X}{:02X} (expected 0x7F454C46)",
            object.elf_header.e_ident[0], object.elf_header.e_ident[1],
            object.elf_header.e_ident[2], object.elf_header.e_ident[3]));
    }

    Util::byteswap(object.elf_header);

    if (object.elf_header.e_machine != EM_PPC) {
        return std::unexpected(std::format(
            "got e_machine value of {} (expected {})", object.elf_header.e_machine, EM_PPC));
    }

    if (object.elf_header.e_ident[EI_CLASS] != ELFCLASS32) {
        return std::unexpected(std::format( //
            "got EI_CLASS value of {} (expected {}). Make sure your binary is 32-bit.",
            object.elf_header.e_ident[EI_CLASS], ELFCLASS32));
    }

    return {};
}

std::expected<void, std::string>
read_sections(Object& object, std::istream& stream) {
    constexpr auto SHT_NOBITS{8uz};

    object.sections.resize(object.elf_header.e_shnum);

    stream.seekg(object.elf_header.e_shoff);
    for (auto [index, section] : Util::enumerate<SectionIndex>(object.sections)) {
        if (!stream.read(reinterpret_cast<char*>(&section.header), sizeof(SectionHeader))) {
            return std::unexpected("reached EOF whilst reading section headers");
        }

        Util::byteswap(section.header);
        section.index = index;
    }

    for (auto& section : object.sections) {
        if (section.header.sh_type == SHT_NOBITS) {
            continue;
        }

        section.data.resize(section.header.sh_size);
        stream.seekg(section.header.sh_offset);
        if (!stream.read(reinterpret_cast<char*>(section.data.data()), section.header.sh_size)) {
            return std::unexpected("reached EOF whilst reading section data");
        }
    }

    return {};
}

std::expected<void, std::string>
read_section_names(Object& object) {
    constexpr auto SHN_UNDEF{0uz};
    constexpr auto SHT_STRTAB{3uz};

    if (object.elf_header.e_shstrndx == SHN_UNDEF) {
        return std::unexpected(std::format( //
            "got string table index of {} (expected non-zero value)", SHN_UNDEF));
    }

    if (object.elf_header.e_shstrndx >= object.sections.size()) {
        return std::unexpected(std::format( //
            "ELF header size ({}) and section header size ({}) do not match",
            object.elf_header.e_shstrndx, object.sections.size()));
    }

    const auto& strtab_section = object.sections[object.elf_header.e_shstrndx];
    if (strtab_section.header.sh_type != SHT_STRTAB) {
        return std::unexpected(std::format("got SHT_STRTAB type flag of {} (expected {})",
            strtab_section.header.sh_type, SHT_STRTAB));
    }

    for (auto& section : object.sections) {
        const auto name = read_string(strtab_section, section.header.sh_name);
        if (!name) {
            return std::unexpected(name.error());
        }

        section.name = *name;
    }

    return {};
}

std::expected<void, std::string>
read_symbols(Object& object) {
    constexpr auto SHT_SYMTAB{2uz};
    constexpr auto SHT_STRTAB{3uz};

    const auto symtab_section = object.get_section(".symtab");
    if (!symtab_section) {
        return std::unexpected("failed to find section .symtab");
    }

    if (symtab_section->header.sh_type != SHT_SYMTAB) {
        return std::unexpected(std::format( //
            "got SHT_SYMTAB type flag of {} (expected {})", //
            symtab_section->header.sh_type, SHT_SYMTAB));
    }

    const auto strtab_index = symtab_section->header.sh_link;
    if (strtab_index >= object.sections.size()) {
        return std::unexpected(std::format( //
            "section .symtab links to section {} (expected <{})", //
            strtab_index, object.sections.size()));
    }

    const auto& strtab_section = object.sections[strtab_index];
    if (strtab_section.header.sh_type != SHT_STRTAB) {
        return std::unexpected(std::format( //
            "got SHT_STRTAB type flag of {} (expected {})", //
            strtab_section.header.sh_type, SHT_STRTAB));
    }

    const auto headers = read_table<SymbolHeader>(*symtab_section);
    if (!headers) {
        return std::unexpected(headers.error());
    }

    object.symbols.reserve(headers->size());
    for (const auto& header : *headers) {
        const auto name = read_string(strtab_section, header.st_name);
        if (!name) {
            return std::unexpected(name.error());
        }

        object.symbols.push_back({//
            .header = header,
            .name = std::string{*name}});
    }

    return {};
}

std::expected<void, std::string>
read_revo_relocations(Object& object) {
    const auto rela_section = object.get_section(".rela.revo_text");
    if (!rela_section) {
        Console::warning("No relocation section found, attempting parse anyway");
        return {};
    }

    auto relocations = read_table<Rela>(*rela_section);
    if (!relocations) {
        return std::unexpected(relocations.error());
    }

    object.revo_relocations = std::move(*relocations);
    return {};
}

std::expected<void, std::string>
read_revo_functions(Object& object) {
    constexpr auto STT_FUNC{2uz};

    const auto input_section = object.get_section(".revo_text");
    if (!input_section) {
        return std::unexpected("failed to get section .revo_text");
    }

    for (const auto& symbol : object.symbols) {
        if (symbol.header.st_shndx != input_section->index) {
            continue;
        }

        if (symbol.type() != STT_FUNC) {
            Console::debug("Skipped symbol {:#x}: type {} (expected {})", //
                symbol.header.st_value, symbol.type(), STT_FUNC);
            continue;
        }

        if (symbol.header.st_value % PPC::INSTRUCTION_SIZE != 0) {
            return std::unexpected(std::format( //
                "function {:#x} is not aligned to {} bytes", //
                symbol.header.st_value, PPC::INSTRUCTION_SIZE));
        }

        if (symbol.header.st_size == 0) {
            return std::unexpected(std::format( //
                "function {:#x} has zero size", symbol.header.st_value));
        }

        if (symbol.header.st_size % PPC::INSTRUCTION_SIZE != 0) {
            return std::unexpected(std::format( //
                "function {:#x} (size of {}) is not a multiple of {} bytes", //
                symbol.header.st_value, symbol.header.st_size, PPC::INSTRUCTION_SIZE));
        }

        const auto bytes = input_section->bytes(symbol.header.st_value, symbol.header.st_size);
        if (!bytes) {
            return std::unexpected(std::format(
                "function {:#x} isn't contained within the input section", symbol.header.st_value));
        }

        std::vector<u32> instructions(symbol.header.st_size / PPC::INSTRUCTION_SIZE);
        std::memcpy(instructions.data(), bytes->data(), bytes->size());
        Util::byteswap(instructions);

        std::flat_map<u32, std::vector<Rela>> relocations;

        for (const auto& relocation : object.revo_relocations) {
            if (!symbol.contains(relocation.r_offset)) {
                continue;
            }

            relocations[relocation.r_offset - symbol.header.st_value].push_back(relocation);
            Console::debug("Relocation {:#x} assigned to function {:#x}", //
                relocation.r_offset, symbol.header.st_value);
        }

        object.revo_functions.push_back({//
            .instructions = std::move(instructions),
            .relocations = std::move(relocations),
            .offset = symbol.header.st_value,
            .size = symbol.header.st_size});
    }

    return {};
}

std::expected<void, std::string>
check_functions(Object& object) {
    std::ranges::sort(object.revo_functions, {}, &Function::offset);

    for (const auto& [previous, next] : std::views::pairwise(object.revo_functions)) {
        if (previous.offset + previous.size > next.offset) {
            return std::unexpected(std::format( //
                "function {:#x} (size of {:#x}) overlaps with function {:#x}", //
                previous.offset, previous.size, next.offset));
        }
    }

    return {};
}

std::expected<void, std::string>
check_relocations(const Object& object) {
    for (const auto& relocation : object.revo_relocations) {
        const auto it = std::ranges::upper_bound(
            object.revo_functions, relocation.r_offset, {}, &Function::offset);

        if (it == object.revo_functions.begin() || !std::prev(it)->contains(relocation.r_offset)) {
            return std::unexpected(std::format(
                "relocation {:#x} isn't referenced by a function", relocation.r_offset));
        }
    }

    return {};
}

std::expected<std::string_view, std::string>
read_string(const Section& section, u32 offset) {
    const std::string_view string_table{
        reinterpret_cast<const char*>(section.data.data()), section.data.size()};

    if (offset >= string_table.size()) {
        return std::unexpected(std::format( //
            "section {} has a string offset of {} (expected <{})", //
            section.index, offset, string_table.size()));
    }

    const auto end = string_table.find('\0', offset);
    return string_table.substr(offset, end - offset);
}

// putting this in the source file is technically bad since you need to have
// this function's definition in the same translation unit as where's it's
// used (aka in the header) but this case it's fine since it's instantiated
// with all reasonable types within this file so if we did testing in some
// other file it'd work.
template <typename TType>
[[nodiscard]] std::expected<std::vector<TType>, std::string>
read_table(const Section& section) {
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

} // namespace Impl

} // namespace Revo::ELF

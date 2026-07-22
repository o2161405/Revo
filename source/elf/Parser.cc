#include "Parser.hh"

#include "elf/ParserResult.hh"
#include "util/Config.hh"
#include "util/Util.hh"

#include <algorithm>

namespace Revo::ELF {

std::expected<ParserResult, std::string>
Parser::parse(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return std::unexpected("failed to open file");
    }

    return parse(stream);
}

std::expected<ParserResult, std::string>
Parser::parse(std::ifstream& stream) {
    ParserResult result;

    return parse_elf_header(result, stream)
        .and_then([&]() { return parse_section_headers(result, stream); })
        .and_then([&]() { return parse_string_table(result, stream); })
        .and_then([&]() { return parse_symbol_table(result, stream); })
        .and_then([&]() { return parse_revo_relocations(result, stream); })
        .and_then([&]() { return parse_revo_functions(result, stream); })
        .transform([&]() { return std::move(result); });
}

std::expected<void, std::string>
Parser::parse_elf_header(ParserResult& result, std::ifstream& stream) {
    constexpr auto ELF_MAGIC = std::to_array<u8>({0x7F, 'E', 'L', 'F'});
    constexpr auto EM_PPC{20uz};
    constexpr auto EI_CLASS{4uz};
    constexpr auto ELFCLASS32{1uz};

    if (!stream.read(reinterpret_cast<char*>(&result.elfHeader), sizeof(result.elfHeader))) {
        return std::unexpected("reached EOF whilst interpreting header");
    }

    // todo: can this formatting be cleaner?
    if (!std::ranges::starts_with(result.elfHeader.e_ident, ELF_MAGIC)) {
        return std::unexpected(
            std::format("Got ELF magic of 0x{:02X}{:02X}{:02X}{:02X} (expected 0x7F454C46)",
                result.elfHeader.e_ident[0], result.elfHeader.e_ident[1],
                result.elfHeader.e_ident[2], result.elfHeader.e_ident[3]));
    }

    Util::byteswap(result.elfHeader);

    if (result.elfHeader.e_machine != EM_PPC) {
        return std::unexpected(std::format(
            "Got e_machine value of {} (expected {})", result.elfHeader.e_machine, EM_PPC));
    }

    // EM_PPC already differentiates between 32 and 64 bit (EM_PPC64),
    // but for the sake of being rigorous EI_CLASS is also checked
    if (result.elfHeader.e_ident[EI_CLASS] != ELFCLASS32) {
        return std::unexpected(
            std::format("Got EI_CLASS value of {} (expected {}). Make sure your input binary is "
                        "32-bit and try again.",
                result.elfHeader.e_ident[EI_CLASS], ELFCLASS32));
    }

    Console::info("Parsed ELF header");
    return {};
}

std::expected<void, std::string>
Parser::parse_section_headers(ParserResult& result, std::ifstream& stream) {
    result.sectionHeaders.reserve(result.elfHeader.e_shnum);

    stream.seekg(result.elfHeader.e_shoff);
    for (auto i{0uz}; i < result.elfHeader.e_shnum; ++i) {
        SectionHeader section_header;
        if (!stream.read(reinterpret_cast<char*>(&section_header), sizeof(SectionHeader))) {
            return std::unexpected("reached EOF whilst reading section headers");
        }

        Util::byteswap(section_header);
        result.sectionHeaders.push_back(section_header);
    }

    Console::info("Parsed {} section headers", result.sectionHeaders.size());
    return {};
}

std::expected<void, std::string>
Parser::parse_string_table(ParserResult& result, std::ifstream& stream) {
    constexpr auto SHN_UNDEF{0uz};
    constexpr auto SHT_STRTAB{3uz};

    if (result.elfHeader.e_shstrndx == SHN_UNDEF) {
        return std::unexpected(
            std::format("Got string table index of {} (expected non-zero value)", SHN_UNDEF));
    }

    if (result.elfHeader.e_shstrndx >= result.sectionHeaders.size()) {
        return std::unexpected(
            std::format("ELF header size ({}) and section header size ({}) do not match",
                result.elfHeader.e_shstrndx, result.sectionHeaders.size()));
    }

    const auto& strtab_header = result.sectionHeaders[result.elfHeader.e_shstrndx];
    if (strtab_header.sh_type != SHT_STRTAB) {
        return std::unexpected(std::format(
            "Got SHT_STRTAB type flag of {} (expected {})", strtab_header.sh_type, SHT_STRTAB));
    }

    result.sectionStringTable.resize(strtab_header.sh_size);
    stream.seekg(strtab_header.sh_offset);
    if (!stream.read(result.sectionStringTable.data(), strtab_header.sh_size)) {
        return std::unexpected("reached EOF whilst reading the string table");
    }

    result.sectionStringTable.push_back('\0');
    Console::info("Parsed string table");

    return {};
}

std::expected<void, std::string>
Parser::parse_symbol_table(ParserResult& result, std::ifstream& stream) {
    constexpr auto SHT_SYMTAB{2uz};

    return get_section(result, ".symtab") //
        .and_then([&](const auto& section) -> std::expected<void, std::string> {
            auto [_, symtab_header] = section;

            if (symtab_header.sh_type != SHT_SYMTAB) {
                return std::unexpected(std::format("Got SHT_SYMTAB type flag of {} (expected {})",
                    symtab_header.sh_type, SHT_SYMTAB));
            }

            if (symtab_header.sh_link == 0 ||
                symtab_header.sh_link >= result.sectionHeaders.size()) {
                return std::unexpected(
                    std::format("Got symbol table index link of {} (expected <{})",
                        symtab_header.sh_link, result.sectionHeaders.size()));
            }

            // Read string table
            const auto& strtab_header = result.sectionHeaders[symtab_header.sh_link];
            result.symbolStringTable.resize(strtab_header.sh_size);
            stream.seekg(strtab_header.sh_offset);
            if (!stream.read(result.symbolStringTable.data(), strtab_header.sh_size)) {
                return std::unexpected("reached EOF whilst reading the symbol string table");
            }
            result.symbolStringTable.push_back('\0');

            // Read symbols
            if (symtab_header.sh_size % sizeof(Symbol) != 0) {
                return std::unexpected(
                    std::format("Got symbol table size of {} (expected multiple of {})",
                        symtab_header.sh_size, sizeof(Symbol)));
            }

            result.symbols.resize(symtab_header.sh_size / sizeof(Symbol));
            stream.seekg(symtab_header.sh_offset);
            if (!stream.read(
                    reinterpret_cast<char*>(result.symbols.data()), symtab_header.sh_size)) {
                return std::unexpected("reached EOF whilst reading the symbol table");
            }

            Util::byteswap(result.symbols);
            Console::info("Parsed {} symbols", result.symbols.size());

            return {};
        });
}

std::expected<void, std::string>
Parser::parse_revo_relocations(ParserResult& result, std::ifstream& stream) {
    return get_section(result, Config::RelaInputSection) //
        .and_then([&](const auto& section) -> std::expected<void, std::string> {
            auto [_, rela_header] = section;

            if (rela_header.sh_size % sizeof(Rela) != 0) {
                return std::unexpected(
                    std::format("Got rela header size of {} (expected multiple of {})",
                        rela_header.sh_size, sizeof(Rela)));
            }

            result.revoRelocations.resize(rela_header.sh_size / sizeof(Rela));
            stream.seekg(rela_header.sh_offset);

            if (!stream.read(
                    reinterpret_cast<char*>(result.revoRelocations.data()), rela_header.sh_size)) {
                return std::unexpected("reached EOF whilst reading rela section");
            }

            Util::byteswap(result.revoRelocations);
            Console::info("Parsed {} Revo relocations", result.revoRelocations.size());

            return {};
        })
        .or_else([](const auto&) -> std::expected<void, std::string> {
            Console::warning("No relocation section found, attempting parse anyway");
            return {};
        });
}

std::expected<void, std::string>
Parser::parse_revo_functions(ParserResult& result, std::ifstream& stream) {
    constexpr auto STT_FUNC{2uz};

    return get_section(result, Config::InputSection) //
        .and_then([&](const auto& section) -> std::expected<void, std::string> {
            auto [input_index, input_header] = section;

            std::vector<bool> assigned_relocations(result.revoRelocations.size(), false);

            for (const auto& symbol : result.symbols) {
                if (symbol.type() != STT_FUNC || symbol.st_shndx != input_index) {
                    Console::debug(
                        "Skipped symbol {:#x}: type {} (expected {}), st_shndx {} (expected {})",
                        symbol.st_value, symbol.type(), STT_FUNC, symbol.st_shndx, input_index);
                    continue;
                }

                if (symbol.st_size == 0) {
                    return std::unexpected(
                        std::format("function {:#x} has zero size", symbol.st_value));
                }

                if (symbol.st_size % sizeof(u32) != 0) {
                    return std::unexpected(
                        std::format("function {:#x} size ({}) is not a multiple of 4 bytes",
                            symbol.st_value, symbol.st_size));
                }

                if (!input_header.contains(symbol)) {
                    return std::unexpected(
                        std::format("function {:#x} isn't contained within the input section",
                            symbol.st_value));
                }

                std::vector<u32> instructions(symbol.st_size / sizeof(u32));

                stream.seekg(input_header.sh_offset + symbol.st_value - input_header.sh_addr);

                if (!stream.read(reinterpret_cast<char*>(instructions.data()), symbol.st_size)) {
                    return std::unexpected("reached EOF whilst reading function bytes");
                }

                Util::byteswap(instructions);

                std::flat_map<u32 /*relative offset*/, std::vector<ELF::Rela>> relocations;

                for (auto [index, rela] : std::views::enumerate(result.revoRelocations)) {
                    if (symbol.contains(rela.r_offset)) {
                        assigned_relocations[index] = true;
                        relocations[rela.r_offset - symbol.st_value].push_back(rela);
                        Console::debug("Relocation {:#x} assigned to function {:#x}", rela.r_offset,
                            symbol.st_value);
                    }
                }

                result.revoFunctions.push_back({//
                    .instructions = std::move(instructions),
                    .relocations = std::move(relocations),
                    .offset = symbol.st_value,
                    .size = symbol.st_size});
            }

            for (auto i{0uz}; i < assigned_relocations.size(); ++i) {
                if (!assigned_relocations[i]) {
                    return std::unexpected(
                        std::format("relocation {:#x} isn't referenced by a function",
                            result.revoRelocations[i].r_offset));
                }
            }

            Console::info("Parsed {} Revo functions", result.revoFunctions.size());

            return {};
        });
}

std::expected<std::pair<Parser::SectionIndex, ELF::SectionHeader>, std::string>
Parser::get_section(const ParserResult& result, std::string_view specified_section) {
    auto it = std::ranges::find_if(
        result.sectionHeaders, [&](const ELF::SectionHeader& section_header) {
            if (section_header.sh_name >= result.sectionStringTable.size()) {
                return false;
            }

            std::string_view section_name{
                result.sectionStringTable.data() + section_header.sh_name};
            return section_name == specified_section;
        });

    if (it != result.sectionHeaders.end()) {
        SectionIndex index = static_cast<SectionIndex>(
            std::distance(result.sectionHeaders.begin(), it));
        return std::pair{index, *it};
    }

    return std::unexpected(std::format("failed to find section {}", specified_section));
}

} // namespace Revo::ELF

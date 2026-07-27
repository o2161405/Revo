#pragma once

#include "elf/Types.hh"

#include <array>
#include <expected>
#include <filesystem>
#include <flat_map>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

namespace Revo::ELF {

class Parser {
public:
    struct Result {
        ELF::ELFHeader elfHeader{};

        std::vector<ELF::SectionHeader> sectionHeaders;
        std::vector<char> sectionStringTable;

        std::vector<ELF::Symbol> symbols;
        std::vector<char> symbolStringTable;

        std::vector<ELF::Rela> revoRelocations;
        std::vector<Function> revoFunctions;
    };

    [[nodiscard]] static std::expected<Parser::Result, std::string>
    parse(const std::filesystem::path& path);

    [[nodiscard]] static std::expected<Parser::Result, std::string>
    parse(std::ifstream& stream);

private:
    Parser() = default;

    // --- Parsing steps ---
    [[nodiscard]] static std::expected<void, std::string>
    parse_elf_header(Parser::Result& result, std::ifstream& stream);

    [[nodiscard]] static std::expected<void, std::string>
    parse_section_headers(Parser::Result& result, std::ifstream& stream);

    [[nodiscard]] static std::expected<void, std::string>
    parse_string_table(Parser::Result& result, std::ifstream& stream);

    [[nodiscard]] static std::expected<void, std::string>
    parse_symbol_table(Parser::Result& result, std::ifstream& stream);

    [[nodiscard]] static std::expected<void, std::string>
    parse_revo_relocations(Parser::Result& result, std::ifstream& stream);

    [[nodiscard]] static std::expected<void, std::string>
    parse_revo_functions(Parser::Result& result, std::ifstream& stream);

    // --- Utility functions ---
    using SectionIndex = u32;
    [[nodiscard]] static std::expected<std::pair<SectionIndex, ELF::SectionHeader>, std::string>
    get_section(const Parser::Result& result, std::string_view section);
};

} // namespace Revo::ELF

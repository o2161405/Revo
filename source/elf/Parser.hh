#pragma once

#include "abstract/FunctionImpl.hh"
#include "elf/ParserResult.hh"
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
    [[nodiscard]] static std::expected<ParserResult, std::string>
    parse(const std::filesystem::path& path);

    [[nodiscard]] static std::expected<ParserResult, std::string>
    parse(std::ifstream& stream);

private:
    Parser() = default;

    // --- Parsing steps ---
    [[nodiscard]] static std::expected<void, std::string>
    parse_elf_header(ParserResult& result, std::ifstream& stream);

    [[nodiscard]] static std::expected<void, std::string>
    parse_section_headers(ParserResult& result, std::ifstream& stream);

    [[nodiscard]] static std::expected<void, std::string>
    parse_string_table(ParserResult& result, std::ifstream& stream);

    [[nodiscard]] static std::expected<void, std::string>
    parse_symbol_table(ParserResult& result, std::ifstream& stream);

    [[nodiscard]] static std::expected<void, std::string>
    parse_revo_relocations(ParserResult& result, std::ifstream& stream);

    [[nodiscard]] static std::expected<void, std::string>
    parse_revo_functions(ParserResult& result, std::ifstream& stream);

    // --- Utility functions ---
    using SectionIndex = u32;
    [[nodiscard]] static std::expected<std::pair<SectionIndex, ELF::SectionHeader>, std::string>
    get_section(const ParserResult& result, std::string_view section);
};

} // namespace Revo::ELF

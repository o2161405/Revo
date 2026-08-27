#pragma once

#include "elf/Types.hh"

#include <array>
#include <expected>
#include <filesystem>
#include <flat_map>
#include <fstream>
#include <string>
#include <vector>

namespace Revo::ELF {

class Parser {
public:
    struct Result {
        ELFHeader elf_header{};
        std::vector<SectionHeader> section_headers;
        std::vector<char> section_string_table;
        std::vector<Symbol> symbols;
        std::vector<char> symbol_string_table;
        std::vector<Rela> revo_relocations;
        std::vector<Function> revo_functions;
    };

    [[nodiscard]] static std::expected<Result, std::string>
    parse(const std::filesystem::path& path);

    [[nodiscard]] static std::expected<Result, std::string>
    parse(std::ifstream& stream);

private:
    explicit Parser(std::ifstream& stream) : mStream(stream) {}

    // Parsing steps
    [[nodiscard]] std::expected<void, std::string>
    read_elf_header();

    [[nodiscard]] std::expected<void, std::string>
    read_section_headers();

    [[nodiscard]] std::expected<void, std::string>
    read_string_table();

    [[nodiscard]] std::expected<void, std::string>
    read_symbol_table();

    [[nodiscard]] std::expected<void, std::string>
    read_revo_relocations();

    [[nodiscard]] std::expected<void, std::string>
    read_revo_functions();

    [[nodiscard]] std::expected<void, std::string>
    check_relocations() const
        pre(std::ranges::is_sorted(mResult.revo_functions, {}, &Function::offset));

    // Utility functions
    using SectionIndex = u16;
    [[nodiscard]] std::expected<std::pair<SectionIndex, SectionHeader>, std::string>
    get_section(std::string_view section) const;

    Result mResult;
    std::ifstream& mStream;
};

} // namespace Revo::ELF

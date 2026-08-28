#pragma once

#include "elf/Section.hh"
#include "elf/Types.hh"

#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Revo::ELF {

class Parser {
public:
    struct Result {
        ELFHeader elf_header{};
        std::vector<Section> sections;
        std::vector<Symbol> symbols;
        std::vector<Rela> revo_relocations;
        std::vector<Function> revo_functions;

        [[nodiscard]] std::optional<const Section&>
        get_section(std::string_view name) const;
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
    read_sections();

    [[nodiscard]] std::expected<void, std::string>
    read_section_names();

    [[nodiscard]] std::expected<void, std::string>
    read_symbol_table();

    [[nodiscard]] std::expected<void, std::string>
    read_revo_relocations();

    [[nodiscard]] std::expected<void, std::string>
    read_revo_functions();

    [[nodiscard]] std::expected<void, std::string>
    check_functions();

    [[nodiscard]] std::expected<void, std::string>
    check_relocations() const;

    // Utility functions
    template <typename TType>
    [[nodiscard]] static std::expected<std::vector<TType>, std::string>
    read_table(const Section& section);

    Result mResult;
    std::ifstream& mStream;
};

} // namespace Revo::ELF

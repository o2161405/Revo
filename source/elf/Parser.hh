#pragma once

#include "elf/Object.hh"

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
    [[nodiscard]] static std::expected<Object, std::string>
    parse(const std::filesystem::path& path);

    [[nodiscard]] static std::expected<Object, std::string>
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
    read_symbols();

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

    [[nodiscard]] static std::expected<std::string_view, std::string>
    read_string(const Section& section, u32 offset);

    Object mObject;
    std::ifstream& mStream;
};

} // namespace Revo::ELF

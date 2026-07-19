#pragma once

#include "elf/Function.hh"
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
    using Function = FunctionImpl<u32 /*raw instruction words*/>;

    [[nodiscard]] const std::vector<Function>&
    functions() const {
        return mRevoFunctions;
    }

    [[nodiscard]] static std::expected<Parser, std::string>
    parse(const std::filesystem::path& path);

    [[nodiscard]] static std::expected<Parser, std::string>
    parse(std::ifstream& stream);

private:
    Parser() = default;

    // --- Parsing steps ---
    [[nodiscard]] std::expected<void, std::string>
    parse_elf_header(std::ifstream& stream);

    [[nodiscard]] std::expected<void, std::string>
    parse_section_headers(std::ifstream& stream);

    [[nodiscard]] std::expected<void, std::string>
    parse_string_table(std::ifstream& stream);

    [[nodiscard]] std::expected<void, std::string>
    parse_symbol_table(std::ifstream& stream);

    [[nodiscard]] std::expected<void, std::string>
    parse_revo_relocations(std::ifstream& stream);

    [[nodiscard]] std::expected<void, std::string>
    parse_revo_functions(std::ifstream& stream);

    // --- Utility functions ---
    using SectionIndex = u32;
    [[nodiscard]] std::expected<std::pair<SectionIndex, ELF::SectionHeader>, std::string>
    get_section(std::string_view section) const;

    // --- Member variables ---
    ELF::ELFHeader mELFHeader{};

    std::vector<ELF::SectionHeader> mSectionHeaders;
    std::vector<char> mSectionStringTable;

    std::vector<ELF::Symbol> mSymbols;
    std::vector<char> mSymbolStringTable;

    std::vector<ELF::Rela> mRevoRelocations;
    std::vector<Function> mRevoFunctions;
};

} // namespace Revo::ELF

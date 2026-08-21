#include "Parser.hh"

#include "ppc/Common.hh"
#include "util/Config.hh"
#include "util/Util.hh"

#include <algorithm>
#include <functional>
#include <ranges>

namespace Revo::ELF {

std::expected<Parser::Result, std::string>
Parser::parse(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return std::unexpected("Failed to open file");
    }

    return parse(stream);
}

std::expected<Parser::Result, std::string>
Parser::parse(std::ifstream& stream) {
    Parser parser(stream);

    return parser.read_elf_header()
        .and_then(std::bind_front(&Parser::read_section_headers, &parser))
        .and_then(std::bind_front(&Parser::read_string_table, &parser))
        .and_then(std::bind_front(&Parser::read_symbol_table, &parser))
        .and_then(std::bind_front(&Parser::read_revo_relocations, &parser))
        .and_then(std::bind_front(&Parser::read_revo_functions, &parser))
        .and_then(std::bind_front(&Parser::check_relocations, &parser))
        .transform([&]() { return std::move(parser.mResult); });
}

std::expected<void, std::string>
Parser::read_elf_header() {
    constexpr auto ELF_MAGIC = std::to_array<u8>({0x7F, 'E', 'L', 'F'});
    constexpr auto EM_PPC{20uz};
    constexpr auto EI_CLASS{4uz};
    constexpr auto ELFCLASS32{1uz};

    if (!mStream.read(reinterpret_cast<char*>(&mResult.elfHeader), sizeof(mResult.elfHeader))) {
        return std::unexpected("reached EOF whilst interpreting header");
    }

    if (!std::ranges::starts_with(mResult.elfHeader.e_ident, ELF_MAGIC)) {
        return std::unexpected(
            std::format("Got ELF magic of 0x{:02X}{:02X}{:02X}{:02X} (expected 0x7F454C46)",
                mResult.elfHeader.e_ident[0], mResult.elfHeader.e_ident[1],
                mResult.elfHeader.e_ident[2], mResult.elfHeader.e_ident[3]));
    }

    Util::byteswap(mResult.elfHeader);

    if (mResult.elfHeader.e_machine != EM_PPC) {
        return std::unexpected(std::format(
            "Got e_machine value of {} (expected {})", mResult.elfHeader.e_machine, EM_PPC));
    }

    if (mResult.elfHeader.e_ident[EI_CLASS] != ELFCLASS32) {
        return std::unexpected(
            std::format("Got EI_CLASS value of {} (expected {}). Make sure your input binary is "
                        "32-bit and try again.",
                mResult.elfHeader.e_ident[EI_CLASS], ELFCLASS32));
    }

    Console::info("Parsed ELF header");
    return {};
}

std::expected<void, std::string>
Parser::read_section_headers() {
    mResult.sectionHeaders.reserve(mResult.elfHeader.e_shnum);

    mStream.seekg(mResult.elfHeader.e_shoff);
    for (auto i{0uz}; i < mResult.elfHeader.e_shnum; ++i) {
        SectionHeader section_header;
        if (!mStream.read(reinterpret_cast<char*>(&section_header), sizeof(SectionHeader))) {
            return std::unexpected("reached EOF whilst reading section headers");
        }

        Util::byteswap(section_header);
        mResult.sectionHeaders.push_back(section_header);
    }

    Console::info("Parsed {} section headers", mResult.sectionHeaders.size());
    return {};
}

std::expected<void, std::string>
Parser::read_string_table() {
    constexpr auto SHN_UNDEF{0uz};
    constexpr auto SHT_STRTAB{3uz};

    if (mResult.elfHeader.e_shstrndx == SHN_UNDEF) {
        return std::unexpected(
            std::format("Got string table index of {} (expected non-zero value)", SHN_UNDEF));
    }

    if (mResult.elfHeader.e_shstrndx >= mResult.sectionHeaders.size()) {
        return std::unexpected(
            std::format("ELF header size ({}) and section header size ({}) do not match",
                mResult.elfHeader.e_shstrndx, mResult.sectionHeaders.size()));
    }

    const auto& strtab_header = mResult.sectionHeaders[mResult.elfHeader.e_shstrndx];
    if (strtab_header.sh_type != SHT_STRTAB) {
        return std::unexpected(std::format(
            "Got SHT_STRTAB type flag of {} (expected {})", strtab_header.sh_type, SHT_STRTAB));
    }

    mResult.sectionStringTable.resize(strtab_header.sh_size);
    mStream.seekg(strtab_header.sh_offset);
    if (!mStream.read(mResult.sectionStringTable.data(), strtab_header.sh_size)) {
        return std::unexpected("reached EOF whilst reading the string table");
    }

    mResult.sectionStringTable.push_back('\0');

    Console::info("Parsed string table");
    return {};
}

std::expected<void, std::string>
Parser::read_symbol_table() {
    constexpr auto SHT_SYMTAB{2uz};

    auto section = get_section(".symtab");
    if (!section) {
        return std::unexpected(section.error());
    }

    auto [_, symtab_header] = *section;

    if (symtab_header.sh_type != SHT_SYMTAB) {
        return std::unexpected(std::format(
            "Got SHT_SYMTAB type flag of {} (expected {})", symtab_header.sh_type, SHT_SYMTAB));
    }

    if (symtab_header.sh_link == 0 || symtab_header.sh_link >= mResult.sectionHeaders.size()) {
        return std::unexpected(std::format("Got symbol table index link of {} (expected <{})",
            symtab_header.sh_link, mResult.sectionHeaders.size()));
    }

    // Read string table
    const auto& strtab_header = mResult.sectionHeaders[symtab_header.sh_link];
    mResult.symbolStringTable.resize(strtab_header.sh_size);
    mStream.seekg(strtab_header.sh_offset);
    if (!mStream.read(mResult.symbolStringTable.data(), strtab_header.sh_size)) {
        return std::unexpected("reached EOF whilst reading the symbol string table");
    }
    mResult.symbolStringTable.push_back('\0');

    // Read symbols
    if (symtab_header.sh_size % sizeof(Symbol) != 0) {
        return std::unexpected(std::format("Got symbol table size of {} (expected multiple of {})",
            symtab_header.sh_size, sizeof(Symbol)));
    }

    mResult.symbols.resize(symtab_header.sh_size / sizeof(Symbol));
    mStream.seekg(symtab_header.sh_offset);
    if (!mStream.read(reinterpret_cast<char*>(mResult.symbols.data()), symtab_header.sh_size)) {
        return std::unexpected("reached EOF whilst reading the symbol table");
    }

    Util::byteswap(mResult.symbols);

    Console::info("Parsed {} symbols", mResult.symbols.size());
    return {};
}

std::expected<void, std::string>
Parser::read_revo_relocations() {
    auto section = get_section(Config::RelaInputSection);
    if (!section) {
        Console::warning("No relocation section found, attempting parse anyway");
        return {};
    }

    auto [_, rela_header] = *section;

    if (rela_header.sh_size % sizeof(Rela) != 0) {
        return std::unexpected(std::format("Got rela header size of {} (expected multiple of {})",
            rela_header.sh_size, sizeof(Rela)));
    }

    mResult.revoRelocations.resize(rela_header.sh_size / sizeof(Rela));
    mStream.seekg(rela_header.sh_offset);
    if (!mStream.read(
            reinterpret_cast<char*>(mResult.revoRelocations.data()), rela_header.sh_size)) {
        return std::unexpected("reached EOF whilst reading rela section");
    }

    Util::byteswap(mResult.revoRelocations);

    Console::info("Parsed {} Revo relocations", mResult.revoRelocations.size());
    return {};
}

std::expected<void, std::string>
Parser::read_revo_functions() {
    constexpr auto STT_FUNC{2uz};

    auto section = get_section(Config::InputSection);
    if (!section) {
        return std::unexpected(section.error());
    }

    auto [input_index, input_header] = *section;

    for (const auto& symbol : mResult.symbols) {
        if (symbol.st_shndx != input_index) {
            continue;
        }

        if (symbol.type() != STT_FUNC) {
            Console::debug("Skipped symbol at {:#x}: type {} (expected {})", symbol.st_value,
                symbol.type(), STT_FUNC);
            continue;
        }

        if (symbol.st_value % PPC::INSTRUCTION_SIZE != 0) {
            return std::unexpected(std::format("function at {:#x} is not aligned to {} bytes",
                symbol.st_value, PPC::INSTRUCTION_SIZE));
        }

        if (symbol.st_size == 0) {
            return std::unexpected(std::format("function at {:#x} has zero size", symbol.st_value));
        }

        if (symbol.st_size % PPC::INSTRUCTION_SIZE != 0) {
            return std::unexpected(
                std::format("function at {:#x} (size of {}) is not a multiple of {} bytes",
                    symbol.st_value, symbol.st_size, PPC::INSTRUCTION_SIZE));
        }

        if (!input_header.contains(symbol)) {
            return std::unexpected(std::format(
                "function at {:#x} isn't contained within the input section", symbol.st_value));
        }

        std::vector<u32> instructions(symbol.st_size / PPC::INSTRUCTION_SIZE);

        mStream.seekg(input_header.sh_offset + symbol.st_value - input_header.sh_addr);
        if (!mStream.read(reinterpret_cast<char*>(instructions.data()), symbol.st_size)) {
            return std::unexpected("reached EOF whilst reading function bytes");
        }

        Util::byteswap(instructions);

        std::flat_map<u32 /*relative offset*/, std::vector<Rela>> relocations;

        for (auto [index, rela] : std::views::enumerate(mResult.revoRelocations)) {
            if (symbol.contains(rela.r_offset)) {
                relocations[rela.r_offset - symbol.st_value].push_back(rela);
                Console::debug(
                    "Relocation {:#x} assigned to function {:#x}", rela.r_offset, symbol.st_value);
            }
        }

        mResult.revoFunctions.push_back({//
            .instructions = std::move(instructions),
            .relocations = std::move(relocations),
            .offset = symbol.st_value,
            .size = symbol.st_size});
    }

    std::ranges::sort(mResult.revoFunctions, {}, &Function::offset);

    for (const auto& [previous, next] : std::views::pairwise(mResult.revoFunctions)) {
        if (previous.offset + previous.size > next.offset) {
            return std::unexpected(
                std::format("function at {:#x} (size of {:#x}) overlaps with function at {:#x}",
                    previous.offset, previous.size, next.offset));
        }
    }

    Console::success("Parsed {} Revo functions", mResult.revoFunctions.size());
    return {};
}

std::expected<void, std::string>
Parser::check_relocations() const {
    for (const auto& rela : mResult.revoRelocations) {
        if (const auto it = std::ranges::upper_bound(
                mResult.revoFunctions, rela.r_offset, {}, &Function::offset);
            it == mResult.revoFunctions.begin() || !std::prev(it)->contains(rela.r_offset)) {
            return std::unexpected(
                std::format("relocation {:#x} isn't referenced by a function", rela.r_offset));
        }
    }

    return {};
}

std::expected<std::pair<Parser::SectionIndex, SectionHeader>, std::string>
Parser::get_section(std::string_view specified_section) const {
    for (auto [index, section_header] : Util::enumerate<SectionIndex>(mResult.sectionHeaders)) {
        if (section_header.sh_name >= mResult.sectionStringTable.size()) {
            continue;
        }

        std::string_view section_name{mResult.sectionStringTable.data() + section_header.sh_name};

        if (section_name == specified_section) {
            return std::pair{index, section_header};
        }
    }

    return std::unexpected(std::format("failed to find section {}", specified_section));
}

} // namespace Revo::ELF

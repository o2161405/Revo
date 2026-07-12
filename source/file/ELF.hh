#pragma once

#include <array>
#include <expected>
#include <filesystem>
#include <flat_map>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

namespace Revo::File {

class ELF {
public:
    using RelativeOffset = u32;

#pragma pack(push, 1)
    struct ELFHeader {
        static constexpr auto IDENT_COUNT{16uz};

        std::array<u8, IDENT_COUNT> e_ident;
        u16 e_type;
        u16 e_machine;
        u32 e_version;
        u32 e_entry;
        u32 e_phoff;
        u32 e_shoff;
        u32 e_flags;
        u16 e_ehsize;
        u16 e_phentsize;
        u16 e_phnum;
        u16 e_shentsize;
        u16 e_shnum;
        u16 e_shstrndx;
    };

    struct Symbol {
        u32 st_name;
        u32 st_value;
        u32 st_size;
        u8 st_info;
        u8 st_other;
        u16 st_shndx;

        [[nodiscard]] constexpr u8
        type() const {
            return st_info & 0xF;
        }

        [[nodiscard]] constexpr u8
        bind() const {
            return st_info >> 4;
        }

        [[nodiscard]] constexpr bool
        contains(u32 address) const {
            return address >= st_value && address < (st_value + st_size);
        }
    };

    struct SectionHeader {
        u32 sh_name;
        u32 sh_type;
        u32 sh_flags;
        u32 sh_addr;
        u32 sh_offset;
        u32 sh_size;
        u32 sh_link;
        u32 sh_info;
        u32 sh_addralign;
        u32 sh_entsize;

        [[nodiscard]] constexpr bool
        contains(const Symbol& symbol) const {
            if (symbol.st_value < sh_addr) {
                return false;
            }

            const RelativeOffset offset = symbol.st_value - sh_addr;
            if (offset >= sh_size || symbol.st_size > sh_size - offset) {
                return false;
            }

            return true;
        }
    };

    struct Rela {
        u32 r_offset;
        u32 r_info;
        s32 r_addend;

        [[nodiscard]] constexpr u32
        symbol_index() const {
            return r_info >> 8;
        }

        [[nodiscard]] constexpr u8
        type() const {
            return r_info & 0xFF;
        }
    };
#pragma pack(pop)

    struct Function {
        std::vector<u32> instructions;
        std::flat_map<RelativeOffset, std::vector<std::reference_wrapper<const Rela>>> relocations;
        u32 offset;
        u32 size;
    };

    [[nodiscard]] static std::expected<ELF, std::string>
    parse(const std::filesystem::path& path);

    [[nodiscard]] static std::expected<ELF, std::string>
    parse(std::ifstream& stream);

private:
    ELF() = default;

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
    [[nodiscard]] std::expected<std::pair<ELF::SectionIndex, ELF::SectionHeader>, std::string>
    get_section(std::string_view specified_section) const;

    // --- Member variables ---
    ELFHeader mELFHeader{};

    std::vector<SectionHeader> mSectionHeaders;
    std::vector<char> mSectionStringTable;

    std::vector<Symbol> mSymbols;
    std::vector<char> mSymbolStringTable;

    std::vector<Rela> mRevoRelocations;
    std::vector<Function> mRevoFunctions;
};

} // namespace Revo::File

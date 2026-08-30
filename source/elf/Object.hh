#pragma once

#include "elf/Section.hh"
#include "elf/Symbol.hh"
#include "elf/Types.hh"

#include <vector>

namespace Revo::ELF {

struct Object {
    ELFHeader elf_header{};
    std::vector<Section> sections;
    std::vector<Symbol> symbols;
    std::vector<Rela> revo_relocations;
    std::vector<Function> revo_functions;

    [[nodiscard]] std::optional<const Section&>
    get_section(std::string_view name) const;

    [[nodiscard]] std::optional<const Symbol&>
    get_symbol(std::string_view name) const;
};

} // namespace Revo::ELF
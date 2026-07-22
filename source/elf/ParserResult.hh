#pragma once

#include "abstract/FunctionImpl.hh"
#include "elf/Types.hh"

namespace Revo {

struct ParserResult {
    ELF::ELFHeader elfHeader{};

    std::vector<ELF::SectionHeader> sectionHeaders;
    std::vector<char> sectionStringTable;

    std::vector<ELF::Symbol> symbols;
    std::vector<char> symbolStringTable;

    std::vector<ELF::Rela> revoRelocations;
    std::vector<FunctionImpl<u32>> revoFunctions;
};

} // namespace Revo

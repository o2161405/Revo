#pragma once

#include <array>

namespace Revo::ELF {

// e_ident
inline constexpr auto ELF_MAGIC = std::to_array<u8>({0x7F, 'E', 'L', 'F'});
inline constexpr auto EI_CLASS{4uz};
inline constexpr u8 ELFCLASS32{0x1};

// e_machine
inline constexpr u16 EM_PPC{0x14};

// e_shstrndx
inline constexpr u16 SHN_UNDEF{0x0};

// sh_type
inline constexpr u32 SHT_SYMTAB{0x2};
inline constexpr u32 SHT_STRTAB{0x3};
inline constexpr u32 SHT_NOBITS{0x8};

// sh_flags
inline constexpr u32 SHF_WRITE{0x1};
inline constexpr u32 SHF_ALLOC{0x2};
inline constexpr u32 SHF_EXECINSTR{0x4};

// st_info
inline constexpr u8 STT_FUNC{0x2};

} // namespace Revo::ELF

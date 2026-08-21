#pragma once

namespace Revo::PPC {

/* clang-format off */

/**
 * @brief Instruction mnemonics
 *
 * Lists the different mnemonics in the PowerPC ISA.
 *
 * @note When adding new mnemonics, ensure the instruction you're adding isn't an
 * alias for another instruction, for example:
 * - `blr` being `bclr` with hardcoded fields
 * - `subic.` being `addic.` with a flipped sign
 *
 * @note If the instruction you're adding *is* an alias, **implement the base
 * instruction instead** if it hasn't been already.
 */
enum class Mnemonic {
    STW,      ///< `stw`
    LWZ,      ///< `lwz`
    BCLR,     ///< `bclr`
    ADD,      ///< `add`
    SUBF,     ///< `subf`
    MULLW,    ///< `mullw`
    ANDI_RC,  ///< `andi.`, there isnt a plain `andi` instruction
    XOR,      ///< `xor`
    OR,       ///< `or`
    RLWINM,   ///< `rlwinm`
    ADDI,     ///< `addi`
    CMPI,     ///< `cmpi`
    BC,       ///< `bc`
    B,        ///< `b`
    CMP,      ///< `cmp`
    MTSPR,    ///< `mtspr`
    ADDIC_RC, ///< `addic.`, seperate opcode from `addic`
    BCCTR,    ///< `bcctr`
    MFSPR,    ///< `mfspr`
    STWU,     ///< `stwu`
    LBZ,      ///< `lbz`
    ADDIS,    ///< `addis`
    MULLI,    ///< `mulli`
    CMPL,     ///< `cmpl`
    CRXOR,    ///< `crxor`
    STB,      ///< `stb`
    ORI,      ///< `ori`
    XORI,     ///< `xori`
    CNTLZW,   ///< `cntlzw`
};

/* clang-format on */

} // namespace Revo::PPC

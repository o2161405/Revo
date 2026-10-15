#pragma once

#include "bindings/Attributes.hh"

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
    STW      [[= Bindings::ReprText{"stw"}    ]], ///< `stw`
    LWZ      [[= Bindings::ReprText{"lwz"}    ]], ///< `lwz`
    BCLR     [[= Bindings::ReprText{"bclr"}   ]], ///< `bclr`
    ADD      [[= Bindings::ReprText{"add"}    ]], ///< `add`
    SUBF     [[= Bindings::ReprText{"subf"}   ]], ///< `subf`
    MULLW    [[= Bindings::ReprText{"mullw"}  ]], ///< `mullw`
    ANDI_RC  [[= Bindings::ReprText{"andi."}  ]], ///< `andi.`, there isnt a plain `andi` instruction
    XOR      [[= Bindings::ReprText{"xor"}    ]], ///< `xor`
    OR       [[= Bindings::ReprText{"or"}     ]], ///< `or`
    RLWINM   [[= Bindings::ReprText{"rlwinm"} ]], ///< `rlwinm`
    ADDI     [[= Bindings::ReprText{"addi"}   ]], ///< `addi`
    CMPI     [[= Bindings::ReprText{"cmpi"}   ]], ///< `cmpi`
    BC       [[= Bindings::ReprText{"bc"}     ]], ///< `bc`
    B        [[= Bindings::ReprText{"b"}      ]], ///< `b`
    CMP      [[= Bindings::ReprText{"cmp"}    ]], ///< `cmp`
    MTSPR    [[= Bindings::ReprText{"mtspr"}  ]], ///< `mtspr`
    ADDIC_RC [[= Bindings::ReprText{"addic."} ]], ///< `addic.`, seperate opcode from `addic`
    BCCTR    [[= Bindings::ReprText{"bcctr"}  ]], ///< `bcctr`
    MFSPR    [[= Bindings::ReprText{"mfspr"}  ]], ///< `mfspr`
    STWU     [[= Bindings::ReprText{"stwu"}   ]], ///< `stwu`
    LBZ      [[= Bindings::ReprText{"lbz"}    ]], ///< `lbz`
    ADDIS    [[= Bindings::ReprText{"addis"}  ]], ///< `addis`
    MULLI    [[= Bindings::ReprText{"mulli"}  ]], ///< `mulli`
    CMPL     [[= Bindings::ReprText{"cmpl"}   ]], ///< `cmpl`
    CRXOR    [[= Bindings::ReprText{"crxor"}  ]], ///< `crxor`
    STB      [[= Bindings::ReprText{"stb"}    ]], ///< `stb`
    ORI      [[= Bindings::ReprText{"ori"}    ]], ///< `ori`
    XORI     [[= Bindings::ReprText{"xori"}   ]], ///< `xori`
    CNTLZW   [[= Bindings::ReprText{"cntlzw"} ]], ///< `cntlzw`
};

/* clang-format on */

} // namespace Revo::PPC

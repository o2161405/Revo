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
    STW      [[= Bindings::Repr::Text{"stw"}    ]], ///< `stw`
    LWZ      [[= Bindings::Repr::Text{"lwz"}    ]], ///< `lwz`
    BCLR     [[= Bindings::Repr::Text{"bclr"}   ]], ///< `bclr`
    ADD      [[= Bindings::Repr::Text{"add"}    ]], ///< `add`
    SUBF     [[= Bindings::Repr::Text{"subf"}   ]], ///< `subf`
    MULLW    [[= Bindings::Repr::Text{"mullw"}  ]], ///< `mullw`
    ANDI_RC  [[= Bindings::Repr::Text{"andi."}  ]], ///< `andi.`, there isnt a plain `andi` instruction
    XOR      [[= Bindings::Repr::Text{"xor"}    ]], ///< `xor`
    OR       [[= Bindings::Repr::Text{"or"}     ]], ///< `or`
    RLWINM   [[= Bindings::Repr::Text{"rlwinm"} ]], ///< `rlwinm`
    ADDI     [[= Bindings::Repr::Text{"addi"}   ]], ///< `addi`
    CMPI     [[= Bindings::Repr::Text{"cmpi"}   ]], ///< `cmpi`
    BC       [[= Bindings::Repr::Text{"bc"}     ]], ///< `bc`
    B        [[= Bindings::Repr::Text{"b"}      ]], ///< `b`
    CMP      [[= Bindings::Repr::Text{"cmp"}    ]], ///< `cmp`
    MTSPR    [[= Bindings::Repr::Text{"mtspr"}  ]], ///< `mtspr`
    ADDIC_RC [[= Bindings::Repr::Text{"addic."} ]], ///< `addic.`, seperate opcode from `addic`
    BCCTR    [[= Bindings::Repr::Text{"bcctr"}  ]], ///< `bcctr`
    MFSPR    [[= Bindings::Repr::Text{"mfspr"}  ]], ///< `mfspr`
    STWU     [[= Bindings::Repr::Text{"stwu"}   ]], ///< `stwu`
    LBZ      [[= Bindings::Repr::Text{"lbz"}    ]], ///< `lbz`
    ADDIS    [[= Bindings::Repr::Text{"addis"}  ]], ///< `addis`
    MULLI    [[= Bindings::Repr::Text{"mulli"}  ]], ///< `mulli`
    CMPL     [[= Bindings::Repr::Text{"cmpl"}   ]], ///< `cmpl`
    CRXOR    [[= Bindings::Repr::Text{"crxor"}  ]], ///< `crxor`
    STB      [[= Bindings::Repr::Text{"stb"}    ]], ///< `stb`
    ORI      [[= Bindings::Repr::Text{"ori"}    ]], ///< `ori`
    XORI     [[= Bindings::Repr::Text{"xori"}   ]], ///< `xori`
    CNTLZW   [[= Bindings::Repr::Text{"cntlzw"} ]], ///< `cntlzw`
};

/* clang-format on */

} // namespace Revo::PPC

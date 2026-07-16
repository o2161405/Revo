#pragma once

//         --- THINGS TO KEEP IN MIND WHEN ADDING MNEMONICS ---
// 1. Instructions that are actually aliases for more complicated
//    instructions (e.g. `blr` actually being a `bclr` with hardcoded
//    fields) are represented as THE BASE INSTRUCTION, do NOT add the
//    simplified ones!!!!

namespace Revo {

enum class Mnemonic {
    STW,
    LWZ,
    BCLR,
};

} // namespace Revo

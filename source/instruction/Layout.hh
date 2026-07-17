#pragma once

#include "instruction/Instruction.hh"
#include "instruction/Operand.hh"

namespace Revo::InstructionLayout {

/* clang-format off */
struct OPCD : Instruction::Field<0, 5, u8>{};

struct IForm {
    struct LI : Instruction::Field<6,  29, s32, Operand::Type::BranchDestination>{};
    struct AA : Instruction::Field<30, 30, u8,  Operand::Type::None, Operand::Behavior::Absolute>{};
    struct LK : Instruction::Field<31, 31, u8,  Operand::Type::None, Operand::Behavior::Link>{};

    using Impl = Instruction::Layout<OPCD, LI, AA, LK>;
};

struct BForm {
    struct BO : Instruction::Field<6,  10, u8,  Operand::Type::Immediate>{};
    struct BI : Instruction::Field<11, 15, u8,  Operand::Type::Immediate>{};
    struct BD : Instruction::Field<16, 29, s16, Operand::Type::BranchDestination>{};
    struct AA : Instruction::Field<30, 30, u8,  Operand::Type::None, Operand::Behavior::Absolute>{};
    struct LK : Instruction::Field<31, 31, u8,  Operand::Type::None, Operand::Behavior::Link>{};

    using Impl = Instruction::Layout<OPCD, BO, BI, BD, AA, LK>;
};

struct SCForm {
    struct LEV   : Instruction::Field<20, 26, u8, Operand::Type::Immediate>{};
    struct BIT30 : Instruction::Field<30, 30, u8>{};

    using Impl = Instruction::Layout<OPCD, LEV, BIT30>;
};

struct DForm {
    struct RT : Instruction::Field<6,  10, u8,  Operand::Type::GPR>{};
    struct RS : Instruction::Field<6,  10, u8,  Operand::Type::GPR>{};
    struct TO : Instruction::Field<6,  10, u8,  Operand::Type::Immediate>{};
    struct BF : Instruction::Field<6,  8,  u8,  Operand::Type::CR>{};
    struct L  : Instruction::Field<10, 10, u8,  Operand::Type::Immediate>{};
    struct RA : Instruction::Field<11, 15, u8,  Operand::Type::GPR>{};
    struct D  : Instruction::Field<16, 31, s16, Operand::Type::Immediate>{};
    struct SI : Instruction::Field<16, 31, s16, Operand::Type::Immediate>{};
    struct UI : Instruction::Field<16, 31, u16, Operand::Type::Immediate>{};

    using Impl_RT_RA_D    = Instruction::Layout<OPCD, RT, RA, D>;
    using Impl_RT_RA_SI   = Instruction::Layout<OPCD, RT, RA, SI>;
    using Impl_RS_RA_D    = Instruction::Layout<OPCD, RS, RA, D>;
    using Impl_RS_RA_UI   = Instruction::Layout<OPCD, RS, RA, UI>;
    using Impl_BF_L_RA_SI = Instruction::Layout<OPCD, BF, L, RA, SI>;
    using Impl_BF_L_RA_UI = Instruction::Layout<OPCD, BF, L, RA, UI>;
    using Impl_TO_RA_SI   = Instruction::Layout<OPCD, TO, RA, SI>;
};

struct XForm {
    struct RT    : Instruction::Field<6,  10, u8, Operand::Type::GPR>{};
    struct RS    : Instruction::Field<6,  10, u8, Operand::Type::GPR>{};
    struct TO    : Instruction::Field<6,  10, u8, Operand::Type::Immediate>{};
    struct BF    : Instruction::Field<6,  8,  u8, Operand::Type::CR>{};
    struct L10   : Instruction::Field<10, 10, u8, Operand::Type::Immediate>{};
    struct RA    : Instruction::Field<11, 15, u8, Operand::Type::GPR>{};
    struct SR    : Instruction::Field<12, 15, u8, Operand::Type::Immediate>{};
    struct RB    : Instruction::Field<16, 20, u8, Operand::Type::GPR>{};
    struct NB    : Instruction::Field<16, 20, u8, Operand::Type::Immediate>{};
    struct SH    : Instruction::Field<16, 20, u8, Operand::Type::Immediate>{};
    struct XO    : Instruction::ExtendedOpcode<21, 30>{};
    struct Rc    : Instruction::Field<31, 31, u8, Operand::Type::None, Operand::Behavior::Record>{};
    struct BIT31 : Instruction::Field<31, 31, u8>{};

    using Impl_RT_RA_RB_XO       = Instruction::Layout<OPCD, RT, RA, RB, XO>;
    using Impl_RT_RA_NB_XO       = Instruction::Layout<OPCD, RT, RA, NB, XO>;
    using Impl_RT_SR_XO          = Instruction::Layout<OPCD, RT, SR, XO>;
    using Impl_RT_RB_XO          = Instruction::Layout<OPCD, RT, RB, XO>;
    using Impl_RT_XO             = Instruction::Layout<OPCD, RT, XO>;
    using Impl_RS_RA_RB_XO_Rc    = Instruction::Layout<OPCD, RS, RA, RB, XO, Rc>;
    using Impl_RS_RA_RB_XO_BIT31 = Instruction::Layout<OPCD, RS, RA, RB, XO, BIT31>;
    using Impl_RS_RA_RB_XO       = Instruction::Layout<OPCD, RS, RA, RB, XO>;
    using Impl_RS_RA_NB_XO       = Instruction::Layout<OPCD, RS, RA, NB, XO>;
    using Impl_RS_RA_SH_XO_Rc    = Instruction::Layout<OPCD, RS, RA, SH, XO, Rc>;
    using Impl_RS_RA_XO_Rc       = Instruction::Layout<OPCD, RS, RA, XO, Rc>;
    using Impl_RS_SR_XO          = Instruction::Layout<OPCD, RS, SR, XO>;
    using Impl_RS_RB_XO          = Instruction::Layout<OPCD, RS, RB, XO>;
    using Impl_RS_XO             = Instruction::Layout<OPCD, RS, XO>;
    using Impl_BF_L10_RA_RB_XO   = Instruction::Layout<OPCD, BF, L10, RA, RB, XO>;
    using Impl_BF_XO             = Instruction::Layout<OPCD, BF, XO>;
    using Impl_TO_RA_RB_XO       = Instruction::Layout<OPCD, TO, RA, RB, XO>;
    using Impl_RA_RB_XO          = Instruction::Layout<OPCD, RA, RB, XO>;
    using Impl_RB_XO             = Instruction::Layout<OPCD, RB, XO>;
    using Impl_XO                = Instruction::Layout<OPCD, XO>;
};

struct XLForm {
    struct BO  : Instruction::Field<6,  10, u8, Operand::Type::Immediate>{};
    struct BT  : Instruction::Field<6,  10, u8, Operand::Type::Immediate>{};
    struct BF  : Instruction::Field<6,  8,  u8, Operand::Type::CR>{};
    struct BI  : Instruction::Field<11, 15, u8, Operand::Type::Immediate>{};
    struct BA  : Instruction::Field<11, 15, u8, Operand::Type::Immediate>{};
    struct BFA : Instruction::Field<11, 13, u8, Operand::Type::CR>{};
    struct BB  : Instruction::Field<16, 20, u8, Operand::Type::Immediate>{};
    struct BH  : Instruction::Field<19, 20, u8, Operand::Type::Immediate>{};
    struct XO  : Instruction::ExtendedOpcode<21, 30>{};
    struct LK  : Instruction::Field<31, 31, u8, Operand::Type::None, Operand::Behavior::Link>{};

    using Impl_BT_BA_BB_XO    = Instruction::Layout<OPCD, BT, BA, BB, XO>;
    using Impl_BO_BI_BH_XO_LK = Instruction::Layout<OPCD, BO, BI, BH, XO, LK>;
    using Impl_BF_BFA_XO      = Instruction::Layout<OPCD, BF, BFA, XO>;
    using Impl_XO             = Instruction::Layout<OPCD, XO>;
};

struct XFXForm {
    struct RT  : Instruction::Field<6,  10, u8,  Operand::Type::GPR>{};
    struct RS  : Instruction::Field<6,  10, u8,  Operand::Type::GPR>{};
    struct SPR : Instruction::Field<11, 20, u16, Operand::Type::SplitImmediate>{};
    struct TBR : Instruction::Field<11, 20, u16, Operand::Type::SplitImmediate>{};
    struct FXM : Instruction::Field<12, 19, u8,  Operand::Type::Immediate>{};
    struct XO  : Instruction::ExtendedOpcode<21, 30> {};

    using Impl_RT_SPR_XO = Instruction::Layout<OPCD, RT, SPR, XO>;
    using Impl_RT_TBR_XO = Instruction::Layout<OPCD, RT, TBR, XO>;
    using Impl_RT_XO     = Instruction::Layout<OPCD, RT, XO>;
    using Impl_RS_FXM_XO = Instruction::Layout<OPCD, RS, FXM, XO>;
    using Impl_RS_SPR_XO = Instruction::Layout<OPCD, RS, SPR, XO>;
};

struct XOForm {
    struct RT : Instruction::Field<6,  10, u8, Operand::Type::GPR>{};
    struct RA : Instruction::Field<11, 15, u8, Operand::Type::GPR>{};
    struct RB : Instruction::Field<16, 20, u8, Operand::Type::GPR>{};
    struct OE : Instruction::Field<21, 21, u8, Operand::Type::None, Operand::Behavior::Overflow>{};
    struct XO : Instruction::ExtendedOpcode<22, 30>{};
    struct Rc : Instruction::Field<31, 31, u8, Operand::Type::None, Operand::Behavior::Record>{};

    using Impl_RT_RA_RB_OE_XO_Rc = Instruction::Layout<OPCD, RT, RA, RB, OE, XO, Rc>;
    using Impl_RT_RA_RB_XO_Rc    = Instruction::Layout<OPCD, RT, RA, RB, XO, Rc>;
    using Impl_RT_RA_OE_XO_Rc    = Instruction::Layout<OPCD, RT, RA, OE, XO, Rc>;
};

struct MForm {
    struct RS : Instruction::Field<6,  10, u8, Operand::Type::GPR>{};
    struct RA : Instruction::Field<11, 15, u8, Operand::Type::GPR>{};
    struct RB : Instruction::Field<16, 20, u8, Operand::Type::GPR>{};
    struct SH : Instruction::Field<16, 20, u8, Operand::Type::Immediate>{};
    struct MB : Instruction::Field<21, 25, u8, Operand::Type::Immediate>{};
    struct ME : Instruction::Field<26, 30, u8, Operand::Type::Immediate>{};
    struct Rc : Instruction::Field<31, 31, u8, Operand::Type::None, Operand::Behavior::Record>{};

    using Impl_RS_RA_RB_MB_ME_Rc = Instruction::Layout<OPCD, RS, RA, RB, MB, ME, Rc>;
    using Impl_RS_RA_SH_MB_ME_Rc = Instruction::Layout<OPCD, RS, RA, SH, MB, ME, Rc>;
};
/* clang-format on */

} // namespace Revo::InstructionLayout

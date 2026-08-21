#pragma once

#include "ppc/Field.hh"
#include "ppc/Instruction.hh"

/**
 * @brief Defines every layout and field used by instructions in the ISA.
 * \cite ibm_powerpc_book1_2005
 * @todo Document the different instruction forms.
 */
namespace Revo::PPC::Layout {

/* clang-format off */
/// \cond
struct OPCD : Field<0, 5, u8>{};
/// \endcond

struct IForm {
    /// \cond
    struct LI : BranchDestinationField<6, 29, s32>{};
    struct AA : BehaviorField<30, 30, Operand::Behavior::Absolute>{};
    struct LK : BehaviorField<31, 31, Operand::Behavior::Link>{};
    
    using Impl = Instruction::Layout<OPCD, LI, AA, LK>;
    /// \endcond
};

struct BForm {
    /// \cond
    struct BO : BranchOptionsField<6, 10>{};
    struct BI : ImmediateField<11, 15, u8>{};
    struct BD : BranchDestinationField<16, 29, s16>{};
    struct AA : BehaviorField<30, 30, Operand::Behavior::Absolute>{};
    struct LK : BehaviorField<31, 31, Operand::Behavior::Link>{};
    
    using Impl = Instruction::Layout<OPCD, BO, BI, BD, AA, LK>;
    /// \endcond
};

struct SCForm {
    /// \cond
    struct LEV   : ImmediateField<20, 26, u8>{};
    struct BIT30 : Field<30, 30, u8>{}; // Raw field, no operand
    
    using Impl = Instruction::Layout<OPCD, LEV, BIT30>;
    /// \endcond
};

struct DForm {
    /// \cond
    struct RT : GPRField<6, 10>{};
    struct RS : GPRField<6, 10>{};
    struct TO : ImmediateField<6, 10, u8>{};
    struct BF : CRField<6, 8>{};
    struct L  : Field<10, 10, u8>{};
    struct RA : GPRField<11, 15>{};
    struct D  : ImmediateField<16, 31, s16>{};
    struct SI : ImmediateField<16, 31, s16>{};
    struct UI : ImmediateField<16, 31, u16>{};

    using Impl_RT_RA_D    = Instruction::Layout<OPCD, RT, RA, D>;
    using Impl_RT_RA_SI   = Instruction::Layout<OPCD, RT, RA, SI>;
    using Impl_RS_RA_D    = Instruction::Layout<OPCD, RS, RA, D>;
    using Impl_RS_RA_UI   = Instruction::Layout<OPCD, RS, RA, UI>;
    using Impl_BF_L_RA_SI = Instruction::Layout<OPCD, BF, L, RA, SI>;
    using Impl_BF_L_RA_UI = Instruction::Layout<OPCD, BF, L, RA, UI>;
    using Impl_TO_RA_SI   = Instruction::Layout<OPCD, TO, RA, SI>;
    /// \endcond
};

struct XForm {
    /// \cond
    struct RT    : GPRField<6, 10>{};
    struct RS    : GPRField<6, 10>{};
    struct TO    : ImmediateField<6, 10, u8>{};
    struct BF    : CRField<6, 8>{};
    struct L10   : Field<10, 10, u8>{}; 
    struct RA    : GPRField<11, 15>{};
    struct SR    : ImmediateField<12, 15, u8>{};
    struct RB    : GPRField<16, 20>{};
    struct NB    : ImmediateField<16, 20, u8>{};
    struct SH    : ImmediateField<16, 20, u8>{};
    struct XO    : ExtendedOpcodeField<21, 30>{};
    struct Rc    : BehaviorField<31, 31, Operand::Behavior::Record>{};
    struct BIT31 : Field<31, 31, u8>{};

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
    /// \endcond
};

struct XLForm {
    /// \cond
    struct BO  : BranchOptionsField<6, 10>{};
    struct BT  : ImmediateField<6, 10, u8>{};
    struct BF  : CRField<6, 8>{};
    struct BI  : ImmediateField<11, 15, u8>{};
    struct BA  : ImmediateField<11, 15, u8>{};
    struct BFA : CRField<11, 13>{};
    struct BB  : ImmediateField<16, 20, u8>{};
    struct BH  : ImmediateField<19, 20, u8>{};
    struct XO  : ExtendedOpcodeField<21, 30>{};
    struct LK  : BehaviorField<31, 31, Operand::Behavior::Link>{};

    using Impl_BT_BA_BB_XO    = Instruction::Layout<OPCD, BT, BA, BB, XO>;
    using Impl_BO_BI_BH_XO_LK = Instruction::Layout<OPCD, BO, BI, BH, XO, LK>;
    using Impl_BF_BFA_XO      = Instruction::Layout<OPCD, BF, BFA, XO>;
    using Impl_XO             = Instruction::Layout<OPCD, XO>;
    /// \endcond
};

struct XFXForm {
    /// \cond
    struct RT  : GPRField<6, 10>{};
    struct RS  : GPRField<6, 10>{};
    struct SPR : SPRField<11, 20>{};
    struct TBR : SplitImmediateField<11, 20>{};
    struct FXM : ImmediateField<12, 19, u8>{};
    struct XO  : ExtendedOpcodeField<21, 30> {};

    using Impl_RT_SPR_XO = Instruction::Layout<OPCD, RT, SPR, XO>;
    using Impl_RT_TBR_XO = Instruction::Layout<OPCD, RT, TBR, XO>;
    using Impl_RT_XO     = Instruction::Layout<OPCD, RT, XO>;
    using Impl_RS_FXM_XO = Instruction::Layout<OPCD, RS, FXM, XO>;
    using Impl_RS_SPR_XO = Instruction::Layout<OPCD, RS, SPR, XO>;
    /// \endcond
};

struct XOForm {
    /// \cond
    struct RT : GPRField<6, 10>{};
    struct RA : GPRField<11, 15>{};
    struct RB : GPRField<16, 20>{};
    struct OE : BehaviorField<21, 21, Operand::Behavior::Overflow>{};
    struct XO : ExtendedOpcodeField<22, 30>{};
    struct Rc : BehaviorField<31, 31, Operand::Behavior::Record>{};

    using Impl_RT_RA_RB_OE_XO_Rc = Instruction::Layout<OPCD, RT, RA, RB, OE, XO, Rc>;
    using Impl_RT_RA_RB_XO_Rc    = Instruction::Layout<OPCD, RT, RA, RB, XO, Rc>;
    using Impl_RT_RA_OE_XO_Rc    = Instruction::Layout<OPCD, RT, RA, OE, XO, Rc>;
    /// \endcond
};

struct MForm {
    /// \cond
    struct RS : GPRField<6, 10>{};
    struct RA : GPRField<11, 15>{};
    struct RB : GPRField<16, 20>{};
    struct SH : ImmediateField<16, 20, u8>{};
    struct MB : ImmediateField<21, 25, u8>{};
    struct ME : ImmediateField<26, 30, u8>{};
    struct Rc : BehaviorField<31, 31, Operand::Behavior::Record>{};

    using Impl_RS_RA_RB_MB_ME_Rc = Instruction::Layout<OPCD, RS, RA, RB, MB, ME, Rc>;
    using Impl_RS_RA_SH_MB_ME_Rc = Instruction::Layout<OPCD, RS, RA, SH, MB, ME, Rc>;
    /// \endcond
};
/* clang-format on */

} // namespace Revo::PPC::Layout

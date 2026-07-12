#pragma once

#include "architecture/Instruction.hh"

namespace Revo::Architecture {

/* clang-format off */
struct Layouts {
    using OPCD = Instruction::Field<0, 5, u8>;

    struct IForm {
        using LI = Instruction::Field<6, 29, s32>;
        using AA = Instruction::Field<30, 30, u8>;
        using LK = Instruction::Field<31, 31, u8>;

        using Impl = Instruction::Layout<OPCD, LI, AA, LK>;
    };

    struct BForm {
        using BO = Instruction::Field<6, 10, u8>;
        using BI = Instruction::Field<11, 15, u8>;
        using BD = Instruction::Field<16, 29, s16>;
        using AA = Instruction::Field<30, 30, u8>;
        using LK = Instruction::Field<31, 31, u8>;

        using Impl = Instruction::Layout<OPCD, BO, BI, BD, AA, LK>;
    };

    struct SCForm {
        using LEV   = Instruction::Field<20, 26, u8>;
        using BIT30 = Instruction::Field<31, 31, u8>;
        
        using Impl = Instruction::Layout<OPCD, LEV, BIT30>;
    };

    struct DForm {
        using RT  = Instruction::Field<6,  10, u8>;
        using RS  = Instruction::Field<6,  10, u8>;
        using TO  = Instruction::Field<6,  10, u8>;
        using FRT = Instruction::Field<6,  10, u8>;
        using FRS = Instruction::Field<6,  10, u8>;
        using BF  = Instruction::Field<6,  8,  u8>;
        using L   = Instruction::Field<10, 10, u8>;
        using RA  = Instruction::Field<11, 15, u8>;
        using D   = Instruction::Field<16, 31, s16>;
        using SI  = Instruction::Field<16, 31, s16>;
        using UI  = Instruction::Field<16, 31, u16>;

        using Impl_RT_RA_D    = Instruction::Layout<OPCD, RT, RA, D>;
        using Impl_RT_RA_SI   = Instruction::Layout<OPCD, RT, RA, SI>;
        using Impl_RS_RA_D    = Instruction::Layout<OPCD, RS, RA, D>;
        using Impl_RS_RA_UI   = Instruction::Layout<OPCD, RS, RA, UI>;
        using Impl_BF_L_RA_SI = Instruction::Layout<OPCD, BF, L, RA, SI>;
        using Impl_BF_L_RA_UI = Instruction::Layout<OPCD, BF, L, RA, UI>;
        using Impl_TO_RA_SI   = Instruction::Layout<OPCD, TO, RA, SI>;
        using Impl_FRT_RA_D   = Instruction::Layout<OPCD, FRT, RA, D>;
        using Impl_FRS_RA_D   = Instruction::Layout<OPCD, FRS, RA, D>;
    };

    struct XForm {
        using RT    = Instruction::Field<6,  10, u8>;
        using RS    = Instruction::Field<6,  10, u8>;
        using TO    = Instruction::Field<6,  10, u8>;
        using FRT   = Instruction::Field<6,  10, u8>;
        using FRS   = Instruction::Field<6,  10, u8>;
        using BT    = Instruction::Field<6,  10, u8>;
        using BF    = Instruction::Field<6,  8,  u8>;
        using TH    = Instruction::Field<7,  10, u8>;
        using L_10  = Instruction::Field<10, 10, u8>;
        using RA    = Instruction::Field<11, 15, u8>;
        using FRA   = Instruction::Field<11, 15, u8>;
        using SR    = Instruction::Field<12, 15, u8>;
        using BFA   = Instruction::Field<11, 13, u8>;
        using L_15  = Instruction::Field<15, 15, u8>;
        using RB    = Instruction::Field<16, 20, u8>;
        using NB    = Instruction::Field<16, 20, u8>;
        using SH    = Instruction::Field<16, 20, u8>;
        using FRB   = Instruction::Field<16, 20, u8>;
        using U     = Instruction::Field<16, 19, u8>;
        using XO    = Instruction::Field<21, 30, u16>;
        using Rc    = Instruction::Field<31, 31, u8>;
        using BIT31 = Instruction::Field<31, 31, u8>;

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
        using Impl_RS_L15_XO         = Instruction::Layout<OPCD, RS, L_15, XO>;
        using Impl_BF_L10_RA_RB_XO   = Instruction::Layout<OPCD, BF, L_10, RA, RB, XO>;
        using Impl_BF_FRA_FRB_XO     = Instruction::Layout<OPCD, BF, FRA, FRB, XO>;
        using Impl_BF_BFA_XO         = Instruction::Layout<OPCD, BF, BFA, XO>;
        using Impl_BF_U_XO_Rc        = Instruction::Layout<OPCD, BF, U, XO, Rc>;
        using Impl_BF_XO             = Instruction::Layout<OPCD, BF, XO>;
        using Impl_TH_RA_RB_XO       = Instruction::Layout<OPCD, TH, RA, RB, XO>;
        using Impl_L10_RA_RB_XO      = Instruction::Layout<OPCD, L_10, RA, RB, XO>;
        using Impl_L10_RB_XO         = Instruction::Layout<OPCD, L_10, RB, XO>;
        using Impl_L10_XO            = Instruction::Layout<OPCD, L_10, XO>;
        using Impl_TO_RA_RB_XO       = Instruction::Layout<OPCD, TO, RA, RB, XO>;
        using Impl_FRT_RA_RB_XO      = Instruction::Layout<OPCD, FRT, RA, RB, XO>;
        using Impl_FRT_FRB_XO_Rc     = Instruction::Layout<OPCD, FRT, FRB, XO, Rc>;
        using Impl_FRT_XO_Rc         = Instruction::Layout<OPCD, FRT, XO, Rc>;
        using Impl_FRS_RA_RB_XO      = Instruction::Layout<OPCD, FRS, RA, RB, XO>;
        using Impl_BT_XO_Rc          = Instruction::Layout<OPCD, BT, XO, Rc>;
        using Impl_RA_RB_XO          = Instruction::Layout<OPCD, RA, RB, XO>;
        using Impl_RB_XO             = Instruction::Layout<OPCD, RB, XO>;
        using Impl_XO                = Instruction::Layout<OPCD, XO>;
    };

    struct XLForm {
        using BO  = Instruction::Field<6,  10, u8>;
        using BT  = Instruction::Field<6,  10, u8>;
        using BF  = Instruction::Field<6,  8,  u8>;
        using BI  = Instruction::Field<11, 15, u8>;
        using BA  = Instruction::Field<11, 15, u8>;
        using BFA = Instruction::Field<11, 13, u8>;
        using BB  = Instruction::Field<16, 20, u8>;
        using BH  = Instruction::Field<19, 20, u8>;
        using XO  = Instruction::Field<21, 30, u16>;
        using LK  = Instruction::Field<31, 31, u8>;

        using Impl_BT_BA_BB_XO    = Instruction::Layout<OPCD, BT, BA, BB, XO>;
        using Impl_BO_BI_BH_XO_LK = Instruction::Layout<OPCD, BO, BI, BH, XO, LK>;
        using Impl_BF_BFA_XO      = Instruction::Layout<OPCD, BF, BFA, XO>;
        using Impl_XO             = Instruction::Layout<OPCD, XO>;
    };

    struct XFXForm {
        using RT  = Instruction::Field<6,  10, u8>;
        using RS  = Instruction::Field<6,  10, u8>;
        using SPR = Instruction::Field<11, 20, u16>;
        using TBR = Instruction::Field<11, 20, u16>;
        using FXM = Instruction::Field<12, 19, u8>;
        using XO  = Instruction::Field<21, 30, u16>;

        using Impl_RT_SPR_XO = Instruction::Layout<OPCD, RT, SPR, XO>;
        using Impl_RT_TBR_XO = Instruction::Layout<OPCD, RT, TBR, XO>;
        using Impl_RT_XO     = Instruction::Layout<OPCD, RT, XO>;
        using Impl_RT_FXM_XO = Instruction::Layout<OPCD, RT, FXM, XO>;
        using Impl_RS_FXM_XO = Instruction::Layout<OPCD, RS, FXM, XO>;
        using Impl_RS_SPR_XO = Instruction::Layout<OPCD, RS, SPR, XO>;
    };

    struct XFLForm {
        using FLM = Instruction::Field<7,  14, u8>;
        using FRB = Instruction::Field<16, 20, u8>;
        using XO  = Instruction::Field<21, 30, u16>;
        using Rc  = Instruction::Field<31, 31, u8>;

        using Impl_FLM_FRB_XO_Rc = Instruction::Layout<OPCD, FLM, FRB, XO, Rc>;
    };

    struct XSForm {
        using RS   = Instruction::Field<6,  10, u8>;
        using RA   = Instruction::Field<11, 15, u8>;
        using SH16 = Instruction::Field<16, 20, u8>;
        using XO   = Instruction::Field<21, 29, u16>;
        using SH30 = Instruction::Field<30, 30, u8>;
        using Rc   = Instruction::Field<31, 31, u8>;

        using Impl_RS_RA_SH_XO_SH30_Rc = Instruction::Layout<OPCD, RS, RA, SH16, XO, SH30, Rc>;
    };

    struct XOForm {
        using RT = Instruction::Field<6,  10, u8>;
        using RA = Instruction::Field<11, 15, u8>;
        using RB = Instruction::Field<16, 20, u8>;
        using OE = Instruction::Field<21, 21, u8>;
        using XO = Instruction::Field<22, 30, u16>;
        using Rc = Instruction::Field<31, 31, u8>;

        using Impl_RT_RA_RB_OE_XO_Rc = Instruction::Layout<OPCD, RT, RA, RB, OE, XO, Rc>;
        using Impl_RT_RA_RB_XO_Rc    = Instruction::Layout<OPCD, RT, RA, RB, XO, Rc>;
        using Impl_RT_RA_OE_XO_Rc    = Instruction::Layout<OPCD, RT, RA, OE, XO, Rc>;
    };

    struct AForm {
        using FRT = Instruction::Field<6,  10, u8>;
        using FRA = Instruction::Field<11, 15, u8>;
        using FRB = Instruction::Field<16, 20, u8>;
        using FRC = Instruction::Field<21, 25, u8>;
        using XO  = Instruction::Field<26, 30, u8>;
        using Rc  = Instruction::Field<31, 31, u8>;

        using Impl_FRT_FRA_FRB_FRC_XO_Rc = Instruction::Layout<OPCD, FRT, FRA, FRB, FRC, XO, Rc>;
        using Impl_FRT_FRA_FRB_XO_Rc     = Instruction::Layout<OPCD, FRT, FRA, FRB, XO, Rc>;
        using Impl_FRT_FRA_FRC_XO_Rc     = Instruction::Layout<OPCD, FRT, FRA, FRC, XO, Rc>;
        using Impl_FRT_FRB_XO_Rc         = Instruction::Layout<OPCD, FRT, FRB, XO, Rc>;
    };

    struct MForm {
        using RS = Instruction::Field<6,  10, u8>;
        using RA = Instruction::Field<11, 15, u8>;
        using RB = Instruction::Field<16, 20, u8>;
        using SH = Instruction::Field<16, 20, u8>;
        using MB = Instruction::Field<21, 25, u8>;
        using ME = Instruction::Field<26, 30, u8>;
        using Rc = Instruction::Field<31, 31, u8>;

        using Impl_RS_RA_RB_MB_ME_Rc = Instruction::Layout<OPCD, RS, RA, RB, MB, ME, Rc>;
        using Impl_RS_RA_SH_MB_ME_Rc = Instruction::Layout<OPCD, RS, RA, SH, MB, ME, Rc>;
    };

    struct MDForm {
        using RS   = Instruction::Field<6,  10, u8>;
        using RA   = Instruction::Field<11, 15, u8>;
        using SH16 = Instruction::Field<16, 20, u8>;
        using MB   = Instruction::Field<21, 26, u8>;
        using ME   = Instruction::Field<21, 26, u8>;
        using XO   = Instruction::Field<27, 29, u8>;
        using SH30 = Instruction::Field<30, 30, u8>;
        using Rc   = Instruction::Field<31, 31, u8>;

        using Impl_RS_RA_SH_MB_XO_SH30_Rc = Instruction::Layout<OPCD, RS, RA, SH16, MB, XO, SH30, Rc>;
        using Impl_RS_RA_SH_ME_XO_SH30_Rc = Instruction::Layout<OPCD, RS, RA, SH16, ME, XO, SH30, Rc>;
    };

    struct MDSForm {
        using RS = Instruction::Field<6,  10, u8>;
        using RA = Instruction::Field<11, 15, u8>;
        using RB = Instruction::Field<16, 20, u8>;
        using MB = Instruction::Field<21, 26, u8>;
        using ME = Instruction::Field<21, 26, u8>;
        using XO = Instruction::Field<27, 30, u8>;
        using Rc = Instruction::Field<31, 31, u8>;

        using Impl_RS_RA_RB_MB_XO_Rc = Instruction::Layout<OPCD, RS, RA, RB, MB, XO, Rc>;
        using Impl_RS_RA_RB_ME_XO_Rc = Instruction::Layout<OPCD, RS, RA, RB, ME, XO, Rc>;
    };
};
/* clang-format on */

} // namespace Revo::Architecture

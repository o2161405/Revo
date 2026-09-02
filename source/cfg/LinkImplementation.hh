#pragma once

#include "cfg/LinkContext.hh"
#include "ppc/Mnemonic.hh"

namespace Revo::CFG {

struct BaseLinkImplementation {
    static void
    apply(LinkContext& context, const Decode::Instruction& instruction) {
        for (const auto& operand : instruction.operands) {
            if (operand.is<PPC::Register::GPR>() &&
                (operand.access & PPC::Operand::Access::Write) != PPC::Operand::Access::None) {
                context.set_lr(operand, false);

                if (operand.is(PPC::Register::GPR::r1)) {
                    context.reset_sp();
                }
            }
        }
    }
};

template <PPC::Mnemonic TMnemonic>
struct LinkImplementation : BaseLinkImplementation {};

template <>
struct LinkImplementation<PPC::Mnemonic::MFSPR> {
    static void
    apply(LinkContext& context, const Decode::Instruction& instruction) {
        const auto [rt, spr] = instruction.get_operands<2uz>();

        context.set_lr(rt, spr.is(PPC::Register::SPR::LR) && context.return_in_lr);
    }
};

template <>
struct LinkImplementation<PPC::Mnemonic::STWU> {
    static void
    apply(LinkContext& context, const Decode::Instruction& instruction) {
        const auto [rs, ra, d] = instruction.get_operands<3uz>();

        if (ra.is(PPC::Register::GPR::r1) && context.current_sp_offset) {
            *context.current_sp_offset += *d.immediate();
            context.store(rs, *context.current_sp_offset);
        }

        context.set_lr(ra, false);
    }
};

template <>
struct LinkImplementation<PPC::Mnemonic::MTSPR> {
    static void
    apply(LinkContext& context, const Decode::Instruction& instruction) {
        const auto [rs, spr] = instruction.get_operands<2uz>();

        if (spr.is(PPC::Register::SPR::LR)) {
            context.return_in_lr = context.has_lr(rs);
        }
    }
};

template <>
struct LinkImplementation<PPC::Mnemonic::STW> {
    static void
    apply(LinkContext& context, const Decode::Instruction& instruction) {
        const auto [rs, ra, d] = instruction.get_operands<3uz>();

        if (ra.is(PPC::Register::GPR::r1) && context.current_sp_offset) {
            context.store(rs, *d.immediate() + *context.current_sp_offset);
        }
    }
};

template <>
struct LinkImplementation<PPC::Mnemonic::LWZ> {
    static void
    apply(LinkContext& context, const Decode::Instruction& instruction) {
        const auto [rt, ra, d] = instruction.get_operands<3uz>();

        context.set_lr(rt,
            ra.is(PPC::Register::GPR::r1) && context.current_sp_offset && //
                context.return_stack_offsets.contains(*d.immediate() + *context.current_sp_offset));
    }
};

template <>
struct LinkImplementation<PPC::Mnemonic::ADDI> {
    static void
    apply(LinkContext& context, const Decode::Instruction& instruction) {
        const auto [rt, ra, si] = instruction.get_operands<3uz>();

        context.set_lr(rt, false);

        if (!rt.is(PPC::Register::GPR::r1)) {
            return;
        }

        if (ra.is(PPC::Register::GPR::r1) && context.current_sp_offset) {
            *context.current_sp_offset += *si.immediate();
        }
        else {
            context.reset_sp();
        }
    }
};

template <>
struct LinkImplementation<PPC::Mnemonic::STB> {
    static void
    apply(LinkContext& context, const Decode::Instruction& instruction) {
        constexpr s32 ALIGNMENT = ~3;

        const auto [_, ra, d] = instruction.get_operands<3uz>();

        if (ra.is(PPC::Register::GPR::r1) && context.current_sp_offset) {
            context.return_stack_offsets.erase(
                (*d.immediate() + *context.current_sp_offset) & ALIGNMENT);
        }
    }
};

} // namespace Revo::CFG

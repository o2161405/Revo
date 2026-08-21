#pragma once

#include "decode/Types.hh"
#include "ppc/Operand.hh"

#include <bitset>
#include <flat_set>
#include <functional>
#include <utility>

namespace Revo::CFG {

struct LinkContext {
    static constexpr u32 CALL_VOLATILE{0b0001111111111001};

    std::flat_set<s32> return_stack_offsets;
    std::optional<s32> current_sp_offset{0};
    std::bitset<32> return_in_gpr{};
    bool return_in_lr{true};

    [[nodiscard]] LinkContext
    merge(this LinkContext result, const LinkContext& other) {
        const auto in_other = [&](s32 offset) { //
            return other.return_stack_offsets.contains(offset);
        };

        result.return_in_gpr &= other.return_in_gpr;
        result.return_in_lr &= other.return_in_lr;

        if (result.current_sp_offset && result.current_sp_offset == other.current_sp_offset) {
            std::erase_if(result.return_stack_offsets, std::not_fn(in_other));
        }
        else {
            result.reset_sp();
        }

        return result;
    }

    [[nodiscard]] bool
    has_lr(const PPC::Operand& operand) const pre(operand.gpr().has_value()) {
        return return_in_gpr.test(std::to_underlying(*operand.gpr()));
    }

    void
    set_lr(const PPC::Operand& operand, bool value) pre(operand.gpr().has_value()) {
        return_in_gpr.set(std::to_underlying(*operand.gpr()), value);
    }

    void
    store(const PPC::Operand& source, s32 offset) {
        if (has_lr(source)) {
            return_stack_offsets.insert(offset);
        }
        else {
            return_stack_offsets.erase(offset);
        }
    }

    void
    reset_sp() {
        current_sp_offset.reset();
        return_stack_offsets.clear();
    }

    void
    clear_offsets() {
        if (current_sp_offset) {
            return_stack_offsets.erase(return_stack_offsets.begin(), //
                return_stack_offsets.lower_bound(*current_sp_offset));
            return_stack_offsets.erase(*current_sp_offset + 4);
        }
    }

    void
    call() {
        return_in_lr = false;
        return_in_gpr &= ~CALL_VOLATILE;
        clear_offsets();
    }

    friend bool
    operator==(const LinkContext&, const LinkContext&) = default;
};

} // namespace Revo::CFG

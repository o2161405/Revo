#pragma once

namespace Revo::Instruction::Register {

/* clang-format off */
enum class GPR : u8 {
    r0 = 0, r1, r2, r3, r4, r5, r6, r7,
    r8, r9, r10, r11, r12, r13, r14, r15,
    r16, r17, r18, r19, r20, r21, r22, r23,
    r24, r25, r26, r27, r28, r29, r30, r31
};

enum class FPR : u8 {
    f0 = 0, f1, f2, f3, f4, f5, f6, f7,
    f8, f9, f10, f11, f12, f13, f14, f15,
    f16, f17, f18, f19, f20, f21, f22, f23,
    f24, f25, f26, f27, f28, f29, f30, f31
};

enum class CR : u8 {
    cr0 = 0, cr1, cr2, cr3, cr4, cr5, cr6, cr7
};
/* clang-format on */

} // namespace Revo::Instruction::Register

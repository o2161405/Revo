    .section .revo_text, "ax", @progbits
    .globl test_common_operations
    .type test_common_operations, @function

test_common_operations:
    add     r3, r3, r4
    subf    r3, r4, r3
    mullw   r5, r3, r3
    andi.   r5, r5, 0xFF
    xor     r6, r4, r3
    slwi    r6, r6, 1
    or      r3, r5, r6
    blr
    .size test_common_operations, .-test_common_operations

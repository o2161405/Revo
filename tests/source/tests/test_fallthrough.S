    .section .revo_text, "ax", @progbits
    .globl test_fallthrough
    .type test_fallthrough, @function

test_fallthrough:
    cmpwi   r3, 0
    beq     .skip
    addi    r3, r3, 1
.skip:
    addi    r3, r3, 10
    blr
    .size test_fallthrough, .-test_fallthrough
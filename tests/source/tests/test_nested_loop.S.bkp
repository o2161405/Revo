    .section .revo_text, "ax", @progbits
    .globl test_nested_loop
    .type test_nested_loop, @function

test_nested_loop:
    li      r5, 0
    mr      r6, r3
.outer:
    mtctr   r4
.inner:
    addi    r5, r5, 1
    bdnz    .inner
    subic.  r6, r6, 1
    bne     .outer
    mr      r3, r5
    blr
    .size test_nested_loop, .-test_nested_loop
    .section .revo_text, "ax", @progbits
    .globl test_unreachable
    .type test_unreachable, @function
test_unreachable:
    cmpwi   r3, 0
    beq     .return
    li      r3, 1
.return:
    blr
.unreachable:
    li      r3, 2
    blr
    .size test_unreachable, .-test_unreachable
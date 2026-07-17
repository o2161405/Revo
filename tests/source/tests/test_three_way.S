    .section .revo_text, "ax", @progbits
    .globl test_three_way
    .type test_three_way, @function

test_three_way:
    cmpwi   r3, 0
    blt     .negative
    bgt     .positive
    li      r3, 0
    b       .return
.negative:
    li      r3, -1
    b       .return
.positive:
    li      r3, 1
.return:
    blr
    .size test_three_way, .-test_three_way
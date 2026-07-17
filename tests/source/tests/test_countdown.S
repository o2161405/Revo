    .globl test_countdown
    .type test_countdown, @function

test_countdown:
    mtctr   r3
.loop:
    bdnz    .loop
    li      r3, 0
    blr
    .size test_countdown, .-test_countdown
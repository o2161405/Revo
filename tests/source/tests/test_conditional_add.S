    .section .revo_text, "ax", @progbits
    .globl test_conditional_add
    .type test_conditional_add, @function

test_conditional_add:
    cmpw    r3, r4
    ble     .less_than_equal
    addi    r3, r3, 100
    b       .return
.less_than_equal:
    add     r3, r3, r4
.return:
    blr
    .size test_conditional_add, .-test_conditional_add

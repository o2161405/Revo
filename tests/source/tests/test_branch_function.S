    .section .revo_text, "ax", @progbits
    .globl test_branch_function
    .type test_branch_function, @function
test_branch_function:
    cmpwi   r3, 0
    bne     .not_zero
    b       test_branch_function_destination
.not_zero:
    li      r3, 0
    blr
    .size test_branch_function, .-test_branch_function

    .globl test_branch_function_destination
    .type test_branch_function_destination, @function
test_branch_function_destination:
    addi    r3, r3, 1
    blr
    .size test_branch_function_destination, .-test_branch_function_destination
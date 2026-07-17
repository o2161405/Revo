    .globl test_sum_array
    .type test_sum_array, @function

test_sum_array:
    cmpwi   r4, 0
    beq     .zero_count
    li      r5, 0
    mtctr   r4
.loop:
    lwz     r6, 0(r3)
    add     r5, r5, r6
    addi    r3, r3, 4
    bdnz    .loop
    mr      r3, r5
    blr
.zero_count:
    li      r3, 0
    blr
    .size test_sum_array, .-test_sum_array

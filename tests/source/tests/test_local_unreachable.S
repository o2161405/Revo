    .section .revo_text, "ax", @progbits
    .globl clear_link_context
    .type clear_link_context, @function
clear_link_context:
    addi    r3, r3, 1
    blr
    .size clear_link_context, .-clear_link_context

    .globl test_local_unreachable
    .type test_local_unreachable, @function
test_local_unreachable:
    mflr    r0
    bl      clear_link_context
    mtlr    r0
    b       .unreachable
    .size test_local_unreachable, .-test_local_unreachable

    .globl test_local_unreachable_implementation
    .type test_local_unreachable_implementation, @function
test_local_unreachable_implementation:
    li      r3, 1
    blr
.unreachable:
    li      r3, 2
    blr
    .size test_local_unreachable_implementation, .-test_local_unreachable_implementation
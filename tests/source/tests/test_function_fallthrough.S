    .section .revo_text, "ax", @progbits
    .globl test_function_fallthrough
    .type test_function_fallthrough, @function
test_function_fallthrough:
    addi    r3, r3, 1
    .size test_function_fallthrough, .-test_function_fallthrough

    .globl test_function_fallthrough_destination
    .type test_function_fallthrough_destination, @function
test_function_fallthrough_destination:
    addi    r3, r3, 2
    blr
    .size test_function_fallthrough_destination, .-test_function_fallthrough_destination
    .section .revo_text, "ax", @progbits
    .globl test_read_write
    .type test_read_write, @function

test_read_write:
    stw     r4, 0(r3)
    lwz     r3, 0(r3)
    blr
    .size test_read_write, .-test_read_write

.section .text.entry
.globl _start

_start:
    la sp, stack_top
    call kmain

1:
    j 1b

.section .bss
.align 12
stack:
    .space 4096
stack_top:

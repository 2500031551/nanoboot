.section .text.entry
.globl _start

_start:
    # OpenSBI has already switched us to Supervisor Mode.
    # Set up our kernel stack.
    la sp, stack_top

    # Start the C kernel.
    call kmain

1:
    j 1b


.section .bss
.align 12

stack:
    .space 4096

stack_top:

.section .text.entry
.globl _start

_start:
    /*
     * Set up kernel stack.
     */
    la sp, stack_top

    /*
     * Clear the BSS section.
     *
     * C global/static variables in .bss
     * must start as zero.
     */
    la t0, __bss_start
    la t1, __bss_end

1:
    bgeu t0, t1, 2f
    sd zero, 0(t0)
    addi t0, t0, 8
    j 1b

2:
    /*
     * Start the C kernel.
     */
    call kmain

3:
    j 3b


.section .bss
.align 12

stack:
    .space 4096

stack_top:


.section .bss
.globl __bss_start
__bss_start:

    .space 0

.globl __bss_end
__bss_end:

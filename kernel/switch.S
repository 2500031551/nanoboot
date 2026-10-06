.section .text

.globl context_switch

/*
 * context_switch(old, new)
 *
 * a0 = address of old context
 * a1 = address of new context
 *
 * Save the current CPU context into old.
 * Restore the next process context from new.
 */
context_switch:

    /*
     * Save current return address.
     */
    sd ra, 0(a0)

    /*
     * Save current stack pointer.
     */
    sd sp, 8(a0)

    /*
     * Save callee-saved registers.
     */
    sd s0, 16(a0)
    sd s1, 24(a0)
    sd s2, 32(a0)
    sd s3, 40(a0)
    sd s4, 48(a0)
    sd s5, 56(a0)
    sd s6, 64(a0)
    sd s7, 72(a0)
    sd s8, 80(a0)
    sd s9, 88(a0)
    sd s10, 96(a0)
    sd s11, 104(a0)


    /*
     * Restore next process return address.
     */
    ld ra, 0(a1)

    /*
     * Restore next process stack pointer.
     */
    ld sp, 8(a1)


    /*
     * Restore callee-saved registers.
     */
    ld s0, 16(a1)
    ld s1, 24(a1)
    ld s2, 32(a1)
    ld s3, 40(a1)
    ld s4, 48(a1)
    ld s5, 56(a1)
    ld s6, 64(a1)
    ld s7, 72(a1)
    ld s8, 80(a1)
    ld s9, 88(a1)
    ld s10, 96(a1)
    ld s11, 104(a1)


    /*
     * Continue execution at the
     * restored process's return address.
     */
    ret


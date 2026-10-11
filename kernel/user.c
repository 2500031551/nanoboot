
/*
 * Parent process user program.
 * PID 1 starts here.
 */
__attribute__((section(".usertext"), aligned(4096), noinline))
void user_program(void)
{
    /* Fill all three child slots; the fourth fork must fail cleanly. */
    asm volatile(
        "li a7, 4\n"
        "ecall\n"
        "li a7, 4\n"
        "ecall\n"
        "li a7, 4\n"
        "ecall\n"
        "li a7, 4\n"
        "ecall\n"
    );

    /* Reap all children so their process-table slots and stacks are freed. */
    asm volatile(
        "li a7, 5\n"
        "ecall\n"
        "li a7, 5\n"
        "ecall\n"
        "li a7, 5\n"
        "ecall\n"
    );

    /* Reuse a released slot, then enter the shell. */
    asm volatile(
        "li a7, 4\n"
        "ecall\n"
        "li a7, 5\n"
        "ecall\n"
        "li a0, 5\n"   /* Sleep for five 10 ms timer ticks. */
        "li a7, 7\n"
        "ecall\n"
        "li a7, 6\n"
        "ecall\n"
    );

    while (1) {
    }
}

/*
 * Child process user program.
 * Each newly created child starts here.
 */
__attribute__((section(".usertext"), aligned(4096), noinline))
void child_program(void)
{
    /* Burn a bounded register-only loop so timer preemption is observable.
     * No stack/data access is used until user pages and user stacks exist.
     */
    asm volatile(
        "li t0, 5000000\n"
        "1: addi t0, t0, -1\n"
        "bnez t0, 1b\n"
        ::: "t0"
    );

    /* System call 1: getpid() */
    asm volatile(
        "li a7, 1\n"
        "ecall\n"
    );

    /* System call 3: exit() */
    asm volatile(
        "li a7, 3\n"
        "ecall\n"
    );

    while (1) {
    }
}

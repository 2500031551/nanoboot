
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

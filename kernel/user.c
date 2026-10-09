
/*
 * Parent process user program.
 * PID 1 starts here.
 */
__attribute__((section(".usertext"), aligned(4096), noinline))
void user_program(void)
{
    /* System call 4: fork() */
    asm volatile(
        "li a7, 4\n"
        "ecall\n"
    );

    /* System call 5: wait() */
    asm volatile(
        "li a7, 5\n"
        "ecall\n"
    );

    while (1) {
    }
}

/*
 * Child process user program.
 * PID 2 starts here.
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

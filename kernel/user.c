/*
 * Parent process user program.
 *
 * PID 1 starts here.
 */
void user_program(void)
{
    /*
     * Call fork().
     * System call number 4.
     */
    asm volatile(
        "li a7, 4\n"
        "ecall\n"
    );

    /*
     * Parent waits for the child.
     *
     * System call number 5.
     */
    asm volatile(
        "li a7, 5\n"
        "ecall\n"
    );

    /*
     * Parent is finished.
     */
    while (1) {
    }
}


/*
 * Child process user program.
 *
 * PID 2 starts here.
 */
void child_program(void)
{
    /*
     * Ask for the child's PID.
     *
     * System call number 1.
     */
    asm volatile(
        "li a7, 1\n"
        "ecall\n"
    );

    /*
     * Child is finished.
     *
     * System call number 3 = exit().
     */
    asm volatile(
        "li a7, 3\n"
        "ecall\n"
    );

    while (1) {
    }
}


void user_program(void)
{
    /*
     * Step 1: Call fork()
     * System call number 4 is placed in a7.
     */
    asm volatile(
        "li a7, 4\n"
        "ecall\n"
    );

    /*
     * Step 2: Call exit()
     * System call number 3.
     */
    asm volatile(
        "li a7, 3\n"
        "ecall\n"
    );

    /*
     * Step 3: Call wait()
     * System call number 5.
     */
    asm volatile(
        "li a7, 5\n"
        "ecall\n"
    );

    /*
     * Stop the user program for now.
     */
    while (1) {
    }
}


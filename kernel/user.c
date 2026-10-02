void user_program(void)
{
    asm volatile(
        "li a7, 3\n"
        "ecall\n"
    );

    while (1) {
    }
}


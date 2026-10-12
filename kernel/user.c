
#include <stdint.h>

static void user_shell(void);
#define USER_TEXT __attribute__((section(".usertext"), noinline))
static USER_TEXT void user_call2(uint64_t number, uint64_t arg0,
                                 uint64_t arg1, uint64_t *result);
static USER_TEXT void user_child_work(void);

/*
 * Parent process user program.
 * PID 1 starts here.
 */
__attribute__((section(".usertext"), aligned(4096), noinline))
void user_program(void)
{
    for (int child = 0; child < 3; child++) {
        uint64_t result;
        user_call2(4, 0, 0, &result);
        if (result == 0) user_child_work();
    }
    uint64_t overflow;
    user_call2(4, 0, 0, &overflow); /* Expected to fail at table capacity. */

    for (int child = 0; child < 3; child++) {
        uint64_t ignored;
        user_call2(5, 0, 0, &ignored);
    }

    uint64_t reused;
    user_call2(4, 0, 0, &reused);
    if (reused == 0) user_child_work();
    uint64_t waited;
    user_call2(5, 0, 0, &waited);
    uint64_t slept;
    user_call2(7, 5, 0, &slept);
    user_shell();
}

static USER_TEXT void user_call2(uint64_t number, uint64_t arg0, uint64_t arg1,
                                 uint64_t *result)
{
    register uint64_t a0 asm("a0") = arg0;
    register uint64_t a1 asm("a1") = arg1;
    register uint64_t a7 asm("a7") = number;
    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");
    *result = a0;
}

static USER_TEXT void user_puts(const char *text)
{
    char local[64];
    while (*text) {
        unsigned long n = 0;
        while (text[n] && n < sizeof(local)) { local[n] = text[n]; n++; }
        uint64_t ignored;
        user_call2(2, (uint64_t)local, n, &ignored);
        text += n;
    }
}

static USER_TEXT int user_equal(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == 0 && *b == 0;
}

static USER_TEXT void user_print_number(uint64_t value)
{
    char digits[20]; char output[20]; unsigned long used = 0;
    if (value == 0) digits[used++] = '0';
    while (value && used < sizeof(digits)) {
        digits[used++] = (char)('0' + value % 10); value /= 10;
    }
    for (unsigned long i = 0; i < used; i++) output[i] = digits[used - i - 1];
    uint64_t ignored;
    user_call2(2, (uint64_t)output, used, &ignored);
}

static USER_TEXT void user_shell(void)
{
    char command[64];
    user_puts("\nNanoBoot user shell (U mode)\nType help for commands.\n");
    for (;;) {
        user_puts("nanoboot> ");
        unsigned long length = 0;
        for (;;) {
            uint64_t c;
            user_call2(8, 0, 0, &c);
            if (c == '\r' || c == '\n') {
                command[length] = 0;
                user_puts("\n");
                break;
            }
            if ((c == 8 || c == 127) && length) {
                length--; user_puts("\b \b"); continue;
            }
            if (c >= 32 && c <= 126 && length < sizeof(command) - 1) {
                command[length++] = (char)c;
                char echo[1] = {(char)c}; uint64_t ignored;
                user_call2(2, (uint64_t)echo, 1, &ignored);
            }
        }
        if (!length) continue;
        if (user_equal(command, "help")) {
            user_puts("Commands: help, hello, prasanthi, whoami, stage, about, ps, getpid, ticks, uptime, uname, pwd, ls, cat <file>, clear, isolation\n");
        } else if (user_equal(command, "hello")) {
            user_puts("Hello from NanoBoot user mode!\n");
        } else if (user_equal(command, "prasanthi") ||
                   user_equal(command, "whoami")) {
            user_puts("Prasanthi Lakshmi (NanoBoot contributor)\n");
        } else if (user_equal(command, "pwd")) {
            user_puts("/ramfs\n");
        } else if (user_equal(command, "clear")) {
            user_puts("\033[2J\033[H");
        } else if (user_equal(command, "getpid")) {
            uint64_t pid; user_call2(1, 0, 0, &pid);
            user_puts("PID "); user_print_number(pid); user_puts("\n");
        } else if (user_equal(command, "isolation")) {
            uint64_t ignored;
            user_call2(9, (uint64_t)command, length, &ignored);
        } else {
            uint64_t ignored;
            user_call2(9, (uint64_t)command, length, &ignored);
        }
    }
}

static USER_TEXT void user_child_work(void)
{
    user_puts("Fork child returned 0\n");
    asm volatile(
        "li t0, 5000000\n"
        "1: addi t0, t0, -1\n"
        "bnez t0, 1b\n"
        ::: "t0"
    );
    uint64_t pid;
    user_call2(1, 0, 0, &pid);
    uint64_t ignored;
    user_call2(3, 0, 0, &ignored);
    for (;;) {}
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

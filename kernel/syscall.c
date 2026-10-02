#include <stdint.h>

#define SYS_GETPID 1
#define SYS_WRITE  2
#define SYS_EXIT   3

volatile uint8_t *uart = (uint8_t *)0x10000000UL;

/* Print a string through the UART */
void print_string(const char *msg)
{
    for (int i = 0; msg[i] != '\0'; i++) {
        *uart = msg[i];
    }
}

/* Print an unsigned integer through the UART */
void print_number(uint64_t number)
{
    if (number == 0) {
        *uart = '0';
        return;
    }

    char digits[20];
    int i = 0;

    while (number > 0) {
        digits[i++] = '0' + (number % 10);
        number /= 10;
    }

    while (i > 0) {
        *uart = digits[--i];
    }
}

/* System call dispatcher */
uint64_t syscall_handler(uint64_t syscall_number)
{
    /* System call 1: getpid() */
    if (syscall_number == SYS_GETPID) {
        print_string("getpid() called\n");

        uint64_t pid = 1;

        print_string("PID = ");
        print_number(pid);
        print_string("\n");

        return pid;
    }

    /* System call 2: write() */
    if (syscall_number == SYS_WRITE) {
        print_string("write() called\n");

        const char *msg = "Hello from user program!\n";
        print_string(msg);

        return 1;
    }

    /* System call 3: exit() */
    if (syscall_number == SYS_EXIT) {
        print_string("exit() called\n");
        print_string("Process finished!\n");

        return 0;
    }

    /* Unknown system call */
    print_string("Unknown system call!\n");

    return 0;
}

#include <stdint.h>

extern void trap_entry(void);
extern uint64_t syscall_handler(uint64_t syscall_number);

void trap_handler(void)
{
    uint64_t cause;

    asm volatile(
        "csrr %0, scause"
        : "=r"(cause)
    );

    /*
     * Print a simple trap message.
     */
    volatile uint8_t *uart =
        (uint8_t *)0x10000000UL;

    const char *msg =
        "Trap occurred! scause = ";

    for (int i = 0; msg[i] != '\0'; i++) {
        *uart = msg[i];
    }

    /*
     * Print scause as hexadecimal.
     */
    const char hex[] = "0123456789ABCDEF";

    *uart = '0';
    *uart = 'x';

    for (int shift = 60; shift >= 0; shift -= 4) {
        *uart = hex[(cause >> shift) & 0xF];
    }

    *uart = '\n';

    /*
     * Cause 8 = Environment call from User mode.
     * Cause 9 = Environment call from Supervisor mode.
     */
    if (cause == 8 || cause == 9) {

        uint64_t syscall_number;

        asm volatile(
            "mv %0, a7"
            : "=r"(syscall_number)
        );

        const char *msg2 =
            "Syscall number received!\n";

        for (int i = 0; msg2[i] != '\0'; i++) {
            *uart = msg2[i];
        }

        uint64_t result =
            syscall_handler(syscall_number);

        asm volatile(
            "mv a0, %0"
            :
            : "r"(result)
        );

        return;
    }

    /*
     * Other traps are not handled yet.
     */
    const char *msg3 =
        "Unhandled trap!\n";

    for (int i = 0; msg3[i] != '\0'; i++) {
        *uart = msg3[i];
    }

    while (1) {
    }
}

void trap_init(void)
{
    uint64_t addr =
        (uint64_t)trap_entry;

    asm volatile(
        "csrw stvec, %0"
        :
        : "r"(addr)
    );
}

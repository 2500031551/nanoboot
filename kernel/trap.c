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

    const char *msg = "Trap occurred!\n";

    volatile uint8_t *uart = (uint8_t *)0x10000000UL;

    for (int i = 0; msg[i] != '\0'; i++) {
        *uart = msg[i];
    }

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

        const char *msg2 = "Syscall number received!\n";

        for (int i = 0; msg2[i] != '\0'; i++) {
            *uart = msg2[i];
        }

        uint64_t result = syscall_handler(syscall_number);

        asm volatile(
            "mv a0, %0"
            :
            : "r"(result)
        );
    }
}

void trap_init(void)
{
    uint64_t addr = (uint64_t)trap_entry;

    asm volatile(
        "csrw stvec, %0"
        :
        : "r"(addr)
    );
}

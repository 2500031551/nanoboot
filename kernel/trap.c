#include <stdint.h>

extern void trap_entry(void);

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

    if (cause == 9) {
        const char *ecall_msg = "S-mode ECALL!\n";

        for (int i = 0; ecall_msg[i] != '\0'; i++) {
            *uart = ecall_msg[i];
        }
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

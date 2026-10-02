#include <stdint.h>

#define UART0 0x10000000UL

extern void trap_init(void);
extern void user_program(void);

static volatile uint8_t *uart = (uint8_t *)UART0;

void kmain(void)
{
    const char *msg = "NanoBoot started!\n";

    for (int i = 0; msg[i] != '\0'; i++) {
        *uart = msg[i];
    }

    trap_init();

    const char *msg2 = "Entering User mode...\n";

    for (int i = 0; msg2[i] != '\0'; i++) {
        *uart = msg2[i];
    }

    /*
     * Set the program counter for sret.
     * user_program() will execute in User mode.
     */
    asm volatile(
        "csrw sepc, %0"
        :
        : "r"(user_program)
    );

    /*
     * Clear SPP in sstatus.
     * SPP = 0 means sret returns to User mode.
     */
    uint64_t sstatus;

    asm volatile(
        "csrr %0, sstatus"
        : "=r"(sstatus)
    );

    sstatus &= ~(1UL << 8);

    asm volatile(
        "csrw sstatus, %0"
        :
        : "r"(sstatus)
    );

    asm volatile("sret");

    while (1) {
    }
}

#include <stdint.h>

extern void user_program(void);

void process_start(void)
{
    /*
     * Start the user program.
     */
    asm volatile(
        "csrw sepc, %0"
        :
        : "r"(user_program)
    );

    /*
     * Switch from Supervisor mode
     * to User mode when sret executes.
     */
    uint64_t sstatus;

    asm volatile(
        "csrr %0, sstatus"
        : "=r"(sstatus)
    );

    /*
     * Clear SPP (bit 8).
     * SPP = 0 means User mode.
     */
    sstatus &= ~(1UL << 8);

    asm volatile(
        "csrw sstatus, %0"
        :
        : "r"(sstatus)
    );

    asm volatile("sret");

    /*
     * Should never reach here.
     */
    while (1) {
    }
}


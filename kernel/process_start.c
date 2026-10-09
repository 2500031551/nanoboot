#include <stdint.h>

extern void child_program(void);

void process_start(void)
{
    /*
     * Start the child process in user mode.
     */
    asm volatile(
        "csrw sepc, %0"
        :
        : "r"(child_program)
    );

    uint64_t sstatus;

    asm volatile(
        "csrr %0, sstatus"
        : "=r"(sstatus)
    );

    /*
     * Clear SPP.
     *
     * SPP = 0 means sret returns to User mode.
     */
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


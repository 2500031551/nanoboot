#include <stdint.h>

extern void trap_entry(void);

void trap_handler(void)
{
    // We will inspect the trap cause here later.
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

#include <stdint.h>

#define UART0 0x10000000UL

static volatile uint8_t *uart = (uint8_t *)UART0;

extern void trap_init(void);

void kmain(void)
{
    const char *msg = "NanoBoot started!\n";

    for (int i = 0; msg[i] != '\0'; i++) {
        *uart = msg[i];
    }

    trap_init();

    const char *msg2 = "Triggering trap...\n";

    for (int i = 0; msg2[i] != '\0'; i++) {
        *uart = msg2[i];
    }

    asm volatile(".word 0");

    const char *msg3 = "Returned from trap!\n";

    for (int i = 0; msg3[i] != '\0'; i++) {
        *uart = msg3[i];
    }

    while (1) {
        // Keep the kernel running.
    }
}

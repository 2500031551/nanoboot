#include <stdint.h>

#define UART0 0x10000000UL

/* Trap system */
extern void trap_init(void);

/* User program */
extern void user_program(void);

/* Process system */
extern void process_init(void);

/* Scheduler */
extern void scheduler_init(void);
extern int scheduler_next(void);

/* Virtual memory */
extern void vm_init(void);
extern void vm_kernel_map(void);
extern void vm_enable(void);

static volatile uint8_t *uart =
    (uint8_t *)UART0;

void kmain(void)
{
    /*
     * Print kernel startup message.
     */
    const char *msg =
        "NanoBoot started!\n";

    for (int i = 0; msg[i] != '\0'; i++) {
        *uart = msg[i];
    }

    /*
     * Initialize process system.
     */
    process_init();

    const char *msg_process =
        "Process system initialized!\n";

    for (int i = 0;
         msg_process[i] != '\0';
         i++) {
        *uart = msg_process[i];
    }

    /*
     * Initialize scheduler.
     */
    scheduler_init();

    int next_pid =
        scheduler_next();

    if (next_pid == -1) {

        const char *msg_scheduler =
            "Scheduler: no READY process\n";

        for (int i = 0;
             msg_scheduler[i] != '\0';
             i++) {
            *uart = msg_scheduler[i];
        }
    }

    /*
     * Initialize virtual memory structures.
     */
    vm_init();

    vm_kernel_map();

    const char *msg_vm =
        "Virtual memory initialized!\n";

    for (int i = 0;
         msg_vm[i] != '\0';
         i++) {
        *uart = msg_vm[i];
    }

    /*
     * Enable Sv39 virtual memory.
     */
    const char *msg_before =
        "Before vm_enable!\n";

    for (int i = 0;
         msg_before[i] != '\0';
         i++) {
        *uart = msg_before[i];
    }

    vm_enable();

    const char *msg_vm_enabled =
        "Sv39 virtual memory enabled!\n";

    for (int i = 0;
         msg_vm_enabled[i] != '\0';
         i++) {
        *uart = msg_vm_enabled[i];
    }

    /*
     * Initialize trap handling.
     */
    trap_init();

    const char *msg2 =
        "Entering User mode...\n";

    for (int i = 0;
         msg2[i] != '\0';
         i++) {
        *uart = msg2[i];
    }

    /*
     * Set the address where sret will continue.
     */
    asm volatile(
        "csrw sepc, %0"
        :
        : "r"(user_program)
    );

    /*
     * Change SPP from Supervisor mode (1)
     * to User mode (0).
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

    /*
     * Return from Supervisor mode.
     * Execution continues at user_program().
     */
    asm volatile("sret");

    /*
     * Should never reach here.
     */
    while (1) {
    }
}

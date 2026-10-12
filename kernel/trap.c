#include <stdint.h>
#include "process.h"
#include "vm.h"

extern void trap_entry(void);
extern uint64_t syscall_handler(uint64_t syscall_number, uint64_t arg0,
                                uint64_t arg1);
extern void timer_on_interrupt(void);
extern uint64_t timer_tick_count(void);

static volatile uint8_t *const uart = (uint8_t *)0x10000000UL;

static void print_text(const char *text)
{
    while (*text != '\0') {
        *uart = (uint8_t)*text++;
    }
}

static void print_hex(uint64_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    *uart = '0';
    *uart = 'x';
    for (int shift = 60; shift >= 0; shift -= 4) {
        *uart = (uint8_t)digits[(value >> shift) & 0xF];
    }
}

static int process_index(int pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid &&
            process_table[i].state != UNUSED) {
            return i;
        }
    }
    return -1;
}

static void save_frame(trapframe_t *destination,
                       const trapframe_t *source)
{
    volatile unsigned long *dst = (volatile unsigned long *)destination;
    const unsigned long *src = (const unsigned long *)source;
    for (unsigned long i = 0;
         i < sizeof(*destination) / sizeof(unsigned long);
         i++) {
        dst[i] = src[i];
    }
}

static void switch_frame_to_pid(trapframe_t *active, int pid)
{
    int index = process_index(pid);
    if (index < 0) {
        print_text("Scheduler returned an invalid PID\n");
        for (;;) {
            __asm__ volatile("wfi");
        }
    }
    save_frame(active, &process_table[index].trapframe);
    vm_switch_process(index);
}

void trap_handler(trapframe_t *frame)
{
    const uint64_t interrupt_bit = 1UL << 63;
    uint64_t cause = frame->scause;

    if (cause & interrupt_bit) {
        uint64_t interrupt_code = cause & ~interrupt_bit;
        if (interrupt_code == 5) { /* Supervisor timer interrupt */
            timer_on_interrupt();
            if (process_wakeup((unsigned long)timer_tick_count()) > 0) {
                print_text("Timer woke a sleeping process\n");
            }
            /* Only preempt user execution. Kernel code is kept atomic for
             * this single-hart scheduler except the shell's safe wait loop.
             */
            if (frame->sstatus & (1UL << 8)) {
                return;
            }

            int previous_pid = process_current_pid();
            int running_index = process_index(process_current_pid());
            if (running_index >= 0) {
                save_frame(&process_table[running_index].trapframe, frame);
                int next_pid = scheduler_next();
                if (next_pid >= 0) {
                    if (next_pid != previous_pid) {
                        print_text("Timer preempt: PID ");
                        print_hex((uint64_t)previous_pid);
                        print_text(" -> PID ");
                        print_hex((uint64_t)next_pid);
                        print_text("\n");
                    }
                    switch_frame_to_pid(frame, next_pid);
                }
            }
            return;
        }

        print_text("Unhandled interrupt: ");
        print_hex(interrupt_code);
        print_text("\n");
        for (;;) {
            __asm__ volatile("wfi");
        }
    }

    /* User and supervisor environment calls. */
    if (cause == 8 || cause == 9) {
        int running_index = process_index(process_current_pid());
        if (running_index >= 0) {
            save_frame(&process_table[running_index].trapframe, frame);
        }

        uint64_t syscall_number = frame->x[17]; /* a7 */
        uint64_t result = syscall_handler(syscall_number, frame->x[10],
                                          frame->x[11]);

        if (running_index >= 0 &&
            process_table[running_index].state == RUNNING) {
            frame->x[10] = result; /* a0 */
            frame->sepc += 4;
            save_frame(&process_table[running_index].trapframe, frame);
            return;
        }

        if (running_index >= 0 &&
            process_table[running_index].state == SLEEPING &&
            result == (uint64_t)-3) {
            /* sleep() resumes after its ecall; wait() retries its ecall. */
            frame->x[10] = 0;
            frame->sepc += 4;
            save_frame(&process_table[running_index].trapframe, frame);
        }

        /* exit() or blocking wait() removed this process from RUNNING. */
        int next_pid = scheduler_next();
        if (next_pid >= 0) {
            switch_frame_to_pid(frame, next_pid);
            return;
        }

        /* All processes may be sleeping. Idle with interrupts enabled so
         * the next timer can wake one and let the scheduler run it.
         */
        for (;;) {
            __asm__ volatile("csrsi sstatus, 2\nwfi\ncsrci sstatus, 2"
                             : : : "memory");
            next_pid = scheduler_next();
            if (next_pid >= 0) {
                switch_frame_to_pid(frame, next_pid);
                return;
            }
        }
    }

    print_text("Unhandled synchronous trap; stopping.\n");
    print_text("sepc=");
    print_hex(frame->sepc);
    print_text(" stval=");
    uint64_t trap_value;
    __asm__ volatile("csrr %0, stval" : "=r"(trap_value));
    print_hex(trap_value);
    print_text(" sstatus=");
    print_hex(frame->sstatus);
    print_text("\n");
    for (;;) {
        __asm__ volatile("wfi");
    }
}

void trap_init(void)
{
    uint64_t addr = (uint64_t)trap_entry;
    __asm__ volatile("csrw stvec, %0" : : "r"(addr));
}

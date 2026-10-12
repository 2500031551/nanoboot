
#include <stdint.h>
#include "process.h"

/* Functions provided by other kernel files */
extern void process_start(void);
extern void context_switch(context_t *old_context,
                           context_t *new_context);
extern unsigned long timer_tick_count(void);

/* Process table */
process_t process_table[MAX_PROCESSES];

/* Clear freestanding kernel data without relying on libc's memset. */
static void clear_words(volatile unsigned long *words,
                        unsigned long word_count)
{
    for (unsigned long i = 0; i < word_count; i++)
    {
        words[i] = 0;
    }
}

/* Initialize all processes */
void process_init(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        process_table[i].pid = 0;
        process_table[i].parent_pid = 0;
        process_table[i].state = UNUSED;
        process_table[i].wake_tick = 0;

        clear_words((volatile unsigned long *)&process_table[i].context,
                    sizeof(context_t) / sizeof(unsigned long));
        clear_words((volatile unsigned long *)&process_table[i].trapframe,
                    sizeof(trapframe_t) / sizeof(unsigned long));
    }

    /* Create the initial process */
    process_table[0].pid = 1;
    process_table[0].parent_pid = 0;
    process_table[0].state = RUNNING;
    process_table[0].wake_tick = 0;
}

/* Return the PID of the process currently marked RUNNING. */
int process_current_pid(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].state == RUNNING)
        {
            return process_table[i].pid;
        }
    }

    return -1;
}

/* Return a process's recorded parent PID, or -1 if it is unknown. */
int process_parent_pid(int pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].pid == pid &&
            process_table[i].state != UNUSED)
        {
            return process_table[i].parent_pid;
        }
    }

    return -1;
}

/* Create a child process */
int process_fork(void)
{
    int parent_pid = process_current_pid();
    int parent_index = parent_pid - 1;
    if (parent_index < 0 || parent_index >= MAX_PROCESSES) return -1;

    for (int i = 1; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].state == UNUSED)
        {
            process_table[i].pid = i + 1;
            process_table[i].parent_pid = parent_pid;
            process_table[i].state = READY;
            process_table[i].wake_tick = 0;

            /* Initialize the child context */
            process_table[i].context.ra =
                (unsigned long)process_start;

            process_table[i].context.sp =
                (unsigned long)(process_table[i].kernel_stack +
                                sizeof(process_table[i].kernel_stack));

            process_table[i].context.s0 = 0;
            process_table[i].context.s1 = 0;
            process_table[i].context.s2 = 0;
            process_table[i].context.s3 = 0;
            process_table[i].context.s4 = 0;
            process_table[i].context.s5 = 0;
            process_table[i].context.s6 = 0;
            process_table[i].context.s7 = 0;
            process_table[i].context.s8 = 0;
            process_table[i].context.s9 = 0;
            process_table[i].context.s10 = 0;
            process_table[i].context.s11 = 0;

            /* Clone the saved user state and its one-page user stack. */
            volatile unsigned long *dst_frame =
                (volatile unsigned long *)&process_table[i].trapframe;
            const unsigned long *src_frame =
                (const unsigned long *)&process_table[parent_index].trapframe;
            for (unsigned long word = 0;
                 word < sizeof(trapframe_t) / sizeof(unsigned long); word++)
                dst_frame[word] = src_frame[word];

            volatile unsigned char *dst_stack = process_table[i].user_stack;
            const unsigned char *src_stack =
                process_table[parent_index].user_stack;
            for (unsigned long byte = 0;
                 byte < sizeof(process_table[i].user_stack); byte++)
                dst_stack[byte] = src_stack[byte];

            uint64_t old_low = (uint64_t)process_table[parent_index].user_stack;
            uint64_t old_high = old_low + sizeof(process_table[parent_index].user_stack);
            uint64_t new_low = (uint64_t)process_table[i].user_stack;
            for (int reg = 0; reg < 32; reg++) {
                uint64_t value = process_table[i].trapframe.x[reg];
                if (value >= old_low && value < old_high)
                    process_table[i].trapframe.x[reg] = new_low + value - old_low;
            }
            volatile unsigned long *stack_words =
                (volatile unsigned long *)process_table[i].user_stack;
            for (unsigned long word = 0;
                 word < sizeof(process_table[i].user_stack) / sizeof(unsigned long);
                 word++) {
                uint64_t value = stack_words[word];
                if (value >= old_low && value < old_high)
                    stack_words[word] = new_low + value - old_low;
            }

            /* Parent sees child PID; child resumes after fork() with zero. */
            process_table[i].trapframe.sepc += 4;
            process_table[i].trapframe.x[10] = 0;

            return process_table[i].pid;
        }
    }

    /* No free process-table entry */
    return -1;
}

/* Mark a process as exited */
void process_exit(int pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].pid == pid &&
            process_table[i].state != UNUSED)
        {
            process_table[i].state = EXITED;
            process_table[i].wake_tick = 0;

            /* Wake a parent blocked in wait(). */
            for (int parent = 0; parent < MAX_PROCESSES; parent++) {
                if (process_table[parent].pid == process_table[i].parent_pid &&
                    process_table[parent].state == SLEEPING &&
                    process_table[parent].wake_tick == 0) {
                    process_table[parent].state = READY;
                }
            }
            return;
        }
    }
}

/* Block the running process until the requested timer tick deadline. */
int process_sleep(int pid, unsigned long ticks)
{
    if (ticks == 0) {
        return 0;
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid &&
            process_table[i].state == RUNNING) {
            process_table[i].wake_tick = timer_tick_count() + ticks;
            process_table[i].state = SLEEPING;
            return 1;
        }
    }
    return -1;
}

/* Called from each timer interrupt to wake expired sleepers. */
int process_wakeup(unsigned long ticks)
{
    int awakened = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == SLEEPING &&
            process_table[i].wake_tick != 0 &&
            process_table[i].wake_tick <= ticks) {
            process_table[i].wake_tick = 0;
            process_table[i].state = READY;
            awakened++;
        }
    }
    return awakened;
}

/* Wait for an exited child and release its table entry */
int process_wait(int parent_pid)
{
    int has_child = 0;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].parent_pid != parent_pid ||
            process_table[i].state == UNUSED) {
            continue;
        }

        has_child = 1;
        if (process_table[i].state == EXITED) {
            int child_pid = process_table[i].pid;

            process_table[i].pid = 0;
            process_table[i].parent_pid = 0;
            process_table[i].state = UNUSED;
            process_table[i].wake_tick = 0;

            clear_words((volatile unsigned long *)&process_table[i].context,
                        sizeof(context_t) / sizeof(unsigned long));
            clear_words((volatile unsigned long *)&process_table[i].trapframe,
                        sizeof(trapframe_t) / sizeof(unsigned long));

            volatile unsigned char *stack = process_table[i].kernel_stack;
            for (unsigned long stack_byte = 0;
                 stack_byte < sizeof(process_table[i].kernel_stack);
                 stack_byte++)
            {
                stack[stack_byte] = 0;
            }

            volatile unsigned char *user_stack = process_table[i].user_stack;
            for (unsigned long stack_byte = 0;
                 stack_byte < sizeof(process_table[i].user_stack);
                 stack_byte++) {
                user_stack[stack_byte] = 0;
            }

            return child_pid;
        }
    }

    if (has_child) {
        int running_pid = process_current_pid();
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].pid == running_pid &&
                process_table[i].state == RUNNING) {
                process_table[i].state = SLEEPING;
                return -2; /* Children exist; caller must block and retry. */
            }
        }
    }

    /* No children to wait for. */
    return -1;
}

/* Initialize scheduler state */
void scheduler_init(void)
{
    /* The initial process is already marked RUNNING. */
}

/* Select the next runnable process */
int scheduler_next(void)
{
    int current = -1;

    /* Find the currently running process */
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].state == RUNNING)
        {
            current = i;
            break;
        }
    }

    /* Search for a READY process */
    for (int offset = 1; offset <= MAX_PROCESSES; offset++)
    {
        int i = (current + offset) % MAX_PROCESSES;

        if (process_table[i].state == READY)
        {
            if (current >= 0)
            {
                process_table[current].state = READY;
            }

            process_table[i].state = RUNNING;
            return process_table[i].pid;
        }
    }

    /* Keep the current process if no other one is ready */
    if (current >= 0)
    {
        return process_table[current].pid;
    }

    return -1;
}

/* Test switching from the parent process to a READY child. */
void test_context_switch(void)
{
    int current_index = -1;
    int next_index = -1;

    /* Find the currently running process. */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == RUNNING) {
            current_index = i;
            break;
        }
    }

    /* Find a READY process. */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == READY) {
            next_index = i;
            break;
        }
    }

    /* Stop if either process is missing. */
    if (current_index < 0 || next_index < 0) {
        return;
    }

    /* Save the current supervisor exception PC. */
    asm volatile(
        "csrr %0, sepc"
        : "=r"(process_table[current_index].trapframe.sepc)
    );

    /* Save the current supervisor status register. */
    asm volatile(
        "csrr %0, sstatus"
        : "=r"(process_table[current_index].trapframe.sstatus)
    );

    /* Update process states. */
    process_table[current_index].state = READY;
    process_table[next_index].state = RUNNING;

    /* Switch saved kernel contexts. */
    context_switch(
        &process_table[current_index].context,
        &process_table[next_index].context
    );
}

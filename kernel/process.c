
#include "process.h"

/* Functions provided by other kernel files */
extern void process_start(void);
extern void child_program(void);
extern void context_switch(context_t *old_context,
                           context_t *new_context);

/* Process table */
process_t process_table[MAX_PROCESSES];

/* Initialize all processes */
void process_init(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        process_table[i].pid = 0;
        process_table[i].parent_pid = 0;
        process_table[i].state = UNUSED;
        process_table[i].wake_tick = 0;

        process_table[i].context.ra = 0;
        process_table[i].context.sp = 0;

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

        process_table[i].trapframe.sepc = 0;
        process_table[i].trapframe.sstatus = 0;
    }

    /* Create the initial process */
    process_table[0].pid = 1;
    process_table[0].parent_pid = 0;
    process_table[0].state = RUNNING;
    process_table[0].wake_tick = 0;
}

/* Create a child process */
int process_fork(void)
{
    for (int i = 1; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].state == UNUSED)
        {
            process_table[i].pid = i + 1;
            process_table[i].parent_pid = 1;
            process_table[i].state = READY;
            process_table[i].wake_tick = 0;

            /* Initialize the child context */
            process_table[i].context.ra =
                (unsigned long)process_start;

            process_table[i].context.sp =
                0x80300000UL +
                ((unsigned long)(i + 1) * 4096UL);

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

            /* Initialize the child's trap frame */
            process_table[i].trapframe.sepc =
                (unsigned long)child_program;

            process_table[i].trapframe.sstatus = 0;

            process_table[i].trapframe.ra = 0;
            process_table[i].trapframe.sp =
                process_table[i].context.sp;
            process_table[i].trapframe.gp = 0;
            process_table[i].trapframe.tp = 0;

            process_table[i].trapframe.t0 = 0;
            process_table[i].trapframe.t1 = 0;
            process_table[i].trapframe.t2 = 0;
            process_table[i].trapframe.t3 = 0;
            process_table[i].trapframe.t4 = 0;
            process_table[i].trapframe.t5 = 0;
            process_table[i].trapframe.t6 = 0;

            process_table[i].trapframe.s0 = 0;
            process_table[i].trapframe.s1 = 0;
            process_table[i].trapframe.s2 = 0;
            process_table[i].trapframe.s3 = 0;
            process_table[i].trapframe.s4 = 0;
            process_table[i].trapframe.s5 = 0;
            process_table[i].trapframe.s6 = 0;
            process_table[i].trapframe.s7 = 0;
            process_table[i].trapframe.s8 = 0;
            process_table[i].trapframe.s9 = 0;
            process_table[i].trapframe.s10 = 0;
            process_table[i].trapframe.s11 = 0;

            process_table[i].trapframe.a0 = 0;
            process_table[i].trapframe.a1 = 0;
            process_table[i].trapframe.a2 = 0;
            process_table[i].trapframe.a3 = 0;
            process_table[i].trapframe.a4 = 0;
            process_table[i].trapframe.a5 = 0;
            process_table[i].trapframe.a6 = 0;
            process_table[i].trapframe.a7 = 0;

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
            return;
        }
    }
}

/* Wait for an exited child and release its table entry */
int process_wait(int parent_pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].parent_pid == parent_pid &&
            process_table[i].state == EXITED)
        {
            int child_pid = process_table[i].pid;

            process_table[i].pid = 0;
            process_table[i].parent_pid = 0;
            process_table[i].state = UNUSED;
            process_table[i].wake_tick = 0;

            process_table[i].context.ra = 0;
            process_table[i].context.sp = 0;

            process_table[i].trapframe.sepc = 0;
            process_table[i].trapframe.sstatus = 0;

            return child_pid;
        }
    }

    /* No exited child found */
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

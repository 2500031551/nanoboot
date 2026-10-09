#include "process.h"

extern void context_switch(
    context_t *old,
    context_t *new
);

extern void process_start(void);
extern void user_program(void);
extern void child_program(void);

process_t process_table[MAX_PROCESSES];

static int scheduler_index = 0;


/*
 * Initialize the process table.
 */
void process_init(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {

        process_table[i].pid = 0;
        process_table[i].parent_pid = 0;
        process_table[i].state = UNUSED;

        /*
         * Clear kernel context.
         */
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

        /*
         * Clear user trapframe.
         */
        process_table[i].trapframe.ra = 0;
        process_table[i].trapframe.sp = 0;

        process_table[i].trapframe.gp = 0;
        process_table[i].trapframe.tp = 0;

        process_table[i].trapframe.t0 = 0;
        process_table[i].trapframe.t1 = 0;
        process_table[i].trapframe.t2 = 0;

        process_table[i].trapframe.s0 = 0;
        process_table[i].trapframe.s1 = 0;

        process_table[i].trapframe.a0 = 0;
        process_table[i].trapframe.a1 = 0;
        process_table[i].trapframe.a2 = 0;
        process_table[i].trapframe.a3 = 0;
        process_table[i].trapframe.a4 = 0;
        process_table[i].trapframe.a5 = 0;
        process_table[i].trapframe.a6 = 0;
        process_table[i].trapframe.a7 = 0;

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

        process_table[i].trapframe.t3 = 0;
        process_table[i].trapframe.t4 = 0;
        process_table[i].trapframe.t5 = 0;
        process_table[i].trapframe.t6 = 0;

        process_table[i].trapframe.sepc = 0;
        process_table[i].trapframe.sstatus = 0;
    }


    /*
     * Create the first process.
     *
     * PID 1 is already running.
     */
    process_table[0].pid = 1;
    process_table[0].parent_pid = 0;
    process_table[0].state = RUNNING;
}


/*
 * Create a child process.
 */
int process_fork(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state == UNUSED) {

            int child_pid = i + 1;

            process_table[i].pid = child_pid;

            process_table[i].parent_pid = 1;

            process_table[i].state = READY;


            /*
             * Initialize child's kernel context.
             *
             * When context_switch() restores
             * this context, execution will begin
             * at process_start().
             */
            process_table[i].context.ra =
                (unsigned long)process_start;


            /*
             * Give the child a separate
             * kernel stack.
             */
            process_table[i].context.sp =
                0x80300000UL +
                ((unsigned long)(i + 1) * 4096UL);


            /*
             * Clear callee-saved registers.
             */
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


            /*
             * Initialize child's user trap state.
             *
             * The child begins at child_program().
             */
            process_table[i].trapframe.sepc =
                (unsigned long)child_program;

            process_table[i].trapframe.sstatus =
                0;


            /*
             * Clear user registers.
             */
            process_table[i].trapframe.ra = 0;
            process_table[i].trapframe.sp = 0;

            process_table[i].trapframe.gp = 0;
            process_table[i].trapframe.tp = 0;

            process_table[i].trapframe.t0 = 0;
            process_table[i].trapframe.t1 = 0;
            process_table[i].trapframe.t2 = 0;

            process_table[i].trapframe.s0 = 0;
            process_table[i].trapframe.s1 = 0;

            process_table[i].trapframe.a0 = 0;
            process_table[i].trapframe.a1 = 0;
            process_table[i].trapframe.a2 = 0;
            process_table[i].trapframe.a3 = 0;
            process_table[i].trapframe.a4 = 0;
            process_table[i].trapframe.a5 = 0;
            process_table[i].trapframe.a6 = 0;
            process_table[i].trapframe.a7 = 0;

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

            process_table[i].trapframe.t3 = 0;
            process_table[i].trapframe.t4 = 0;
            process_table[i].trapframe.t5 = 0;
            process_table[i].trapframe.t6 = 0;

            return child_pid;
        }
    }

    return -1;
}


/*
 * Exit a process.
 */
void process_exit(int pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].pid == pid &&
            process_table[i].state != UNUSED) {

            process_table[i].state = EXITED;

            return;
        }
    }
}


/*
 * Wait for an exited child.
 */
int process_wait(int parent_pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].parent_pid == parent_pid &&
            process_table[i].state == EXITED) {

            int child_pid =
                process_table[i].pid;


            /*
             * Recycle the process table entry.
             */
            process_table[i].pid = 0;
            process_table[i].parent_pid = 0;
            process_table[i].state = UNUSED;


            /*
             * Clear kernel context.
             */
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

            return child_pid;
        }
    }

    return -1;
}


/*
 * Initialize scheduler.
 */
void scheduler_init(void)
{
    scheduler_index = 0;
}


/*
 * Select the next READY process.
 *
 * This currently selects the next process
 * but does not perform the real CPU switch.
 */
int scheduler_next(void)
{
    int current_index = -1;
    int next_index = -1;


    /*
     * Find currently running process.
     */
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state == RUNNING) {

            current_index = i;

            break;
        }
    }


    /*
     * Search for next READY process.
     */
    for (int count = 1;
         count <= MAX_PROCESSES;
         count++) {

        int index =
            (scheduler_index + count)
            % MAX_PROCESSES;


        if (process_table[index].state == READY) {

            next_index = index;

            break;
        }
    }


    /*
     * No READY process.
     */
	if (next_index < 0) {
        if (current_index >= 0) {
            return process_table[current_index].pid;
        }

        return -1;
    }


    /*
     * Change process states.
     */
    if (current_index >= 0) {

        process_table[current_index].state =
            READY;
    }


    process_table[next_index].state =
        RUNNING;


    scheduler_index =
        (next_index + 1) % MAX_PROCESSES;


    return process_table[next_index].pid;
}


/*
 * Experimental context-switch test.
 */
void test_context_switch(void)
{
    int current_index = -1;
    int next_index = -1;


    /*
     * Find current process.
     */
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state == RUNNING) {

            current_index = i;

            break;
        }
    }


    /*
     * Find READY process.
     */
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state == READY) {

            next_index = i;

            break;
        }
    }


    /*
     * Make sure both exist.
     */
    if (current_index < 0 ||
        next_index < 0) {

        return;
    }


    /*
     * Save the parent's current trap PC.
     *
     * The parent is currently inside
     * the fork() system call.
     */
    asm volatile(
        "csrr %0, sepc"
        : "=r"(process_table[current_index].trapframe.sepc)
    );


    /*
     * Save the parent's status register.
     */
    asm volatile(
        "csrr %0, sstatus"
        : "=r"(process_table[current_index].trapframe.sstatus)
    );


    /*
     * Change states.
     */
    process_table[current_index].state =
        READY;

    process_table[next_index].state =
        RUNNING;


    /*
     * Perform kernel context switch.
     */
    context_switch(
        &process_table[current_index].context,
        &process_table[next_index].context
    );
}


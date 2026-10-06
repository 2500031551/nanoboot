
#include "process.h"

extern void context_switch(
    context_t *old,
    context_t *new
);

extern void process_start(void);

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
         * Clear the saved CPU context.
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
    }


    /*
     * Create the first process.
     *
     * PID 1 is the currently running
     * kernel/user process.
     */
    process_table[0].pid = 1;
    process_table[0].parent_pid = 0;
    process_table[0].state = RUNNING;

    /*
     * We do NOT assign a fake stack or
     * return address to the currently
     * running process.
     *
     * Its real CPU registers are already
     * active.
     */
}


/*
 * Create a child process.
 */
int process_fork(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state == UNUSED) {

            process_table[i].pid = i + 1;

            process_table[i].parent_pid = 1;

            process_table[i].state = READY;


            /*
             * Initialize the child's CPU context.
             *
             * When the scheduler switches
             * to this process, execution will
             * begin at process_start().
             */
            process_table[i].context.ra =
                (unsigned long)process_start;


            /*
             * Give the child its own stack.
             *
             * These addresses are inside
             * the kernel's currently mapped
             * 0x80200000 - 0x80400000 area.
             */
            process_table[i].context.sp =
                (unsigned long)(
                    0x80300000UL +
                    ((i + 1) * 4096)
                );


            /*
             * Clear the child's
             * callee-saved registers.
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


            return process_table[i].pid;
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

        if (process_table[i].pid == pid) {

            process_table[i].state =
                EXITED;

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
             * Clear the saved context.
             */
            process_table[i].context.ra = 0;
            process_table[i].context.sp = 0;

            return child_pid;
        }
    }

    return -1;
}


/*
 * Initialize the scheduler.
 */
void scheduler_init(void)
{
    scheduler_index = 0;
}


/*
 * Select the next READY process.
 *
 * Stage 7 currently prepares the
 * context switch but does not perform
 * the actual switch yet.
 */
int scheduler_next(void)
{
    int current_index = -1;


    /*
     * Find the currently running process.
     */
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state == RUNNING) {

            current_index = i;

            break;
        }
    }


    /*
     * Search for the next READY process.
     */
    for (int count = 1;
         count <= MAX_PROCESSES;
         count++) {

        int index =
            (scheduler_index + count)
            % MAX_PROCESSES;


        if (process_table[index].state == READY) {

            /*
             * A process was found.
             */
            if (current_index >= 0) {

                /*
                 * Keep the current process
                 * ready for another turn.
                 */
                process_table[current_index].state =
                    READY;


                /*
                 * Mark the selected process
                 * as running.
                 */
                process_table[index].state =
                    RUNNING;


                scheduler_index =
                    (index + 1) % MAX_PROCESSES;


                /*
                 * IMPORTANT:
                 *
                 * The actual context_switch()
                 * is intentionally NOT called yet.
                 *
                 * We first need to correctly
                 * save the parent's real CPU
                 * context.
                 */
                return process_table[index].pid;
            }


            /*
             * No current process exists.
             */
            process_table[index].state =
                RUNNING;

            scheduler_index =
                (index + 1) % MAX_PROCESSES;

            return process_table[index].pid;
        }
    }


    /*
     * No READY process found.
     */
    return -1;
}


#include "process.h"

process_t process_table[MAX_PROCESSES];

/*
 * Keeps track of where the scheduler should start searching.
 */
static int scheduler_index = 0;

void process_init(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].parent_pid = 0;
        process_table[i].state = UNUSED;
    }

    /*
     * Create the first process.
     * PID 1 is the initial NanoBoot process.
     * It has no parent.
     */
    process_table[0].pid = 1;
    process_table[0].parent_pid = 0;
    process_table[0].state = RUNNING;
}

int process_fork(void)
{
    /*
     * Find an unused process slot.
     */
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state == UNUSED) {

            /*
             * Create a new child process.
             */
            process_table[i].pid = i + 1;

            /*
             * PID 1 is the parent.
             */
            process_table[i].parent_pid = 1;

            process_table[i].state = READY;

            /*
             * Return the child's PID.
             */
            return process_table[i].pid;
        }
    }

    /*
     * No free process slot.
     */
    return -1;
}

void process_exit(int pid)
{
    /*
     * Find the process with this PID.
     */
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].pid == pid) {

            /*
             * Mark the process as exited.
             */
            process_table[i].state = EXITED;

            return;
        }
    }
}

int process_wait(int parent_pid)
{
    /*
     * Look for a child belonging to the parent.
     */
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].parent_pid == parent_pid &&
            process_table[i].state == EXITED) {

            /*
             * Return the child's PID.
             */
            int child_pid = process_table[i].pid;

            /*
             * Reclaim the process slot.
             */
            process_table[i].pid = 0;
            process_table[i].parent_pid = 0;
            process_table[i].state = UNUSED;

            return child_pid;
        }
    }

    /*
     * No exited child found.
     */
    return -1;
}

/*
 * Initialize the round-robin scheduler.
 */
void scheduler_init(void)
{
    scheduler_index = 0;
}

/*
 * Select the next READY process.
 *
 * Returns:
 *   PID of the selected process
 *   -1 if no READY process exists
 */
int scheduler_next(void)
{
    for (int count = 0; count < MAX_PROCESSES; count++) {

        int index = (scheduler_index + count) % MAX_PROCESSES;

        if (process_table[index].state == READY) {

            /*
             * Move the scheduler position forward.
             */
            scheduler_index = (index + 1) % MAX_PROCESSES;

            /*
             * Mark this process as running.
             */
            process_table[index].state = RUNNING;

            return process_table[index].pid;
        }
    }

    /*
     * No READY process found.
     */
    return -1;
}

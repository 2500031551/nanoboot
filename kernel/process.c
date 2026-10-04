#include "process.h"

process_t process_table[MAX_PROCESSES];

void process_init(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = UNUSED;
    }

    /*
     * Create the first process.
     * PID 1 is the initial NanoBoot process.
     */
    process_table[0].pid = 1;
    process_table[0].state = RUNNING;
}

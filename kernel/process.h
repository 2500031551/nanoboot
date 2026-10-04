#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 4

typedef enum {
    UNUSED,
    READY,
    RUNNING,
    EXITED
} process_state_t;

typedef struct {
    int pid;
    process_state_t state;
} process_t;

extern process_t process_table[MAX_PROCESSES];

void process_init(void);

#endif

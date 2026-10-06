#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 4

typedef enum {
    UNUSED,
    READY,
    RUNNING,
    EXITED
} process_state_t;

/*
 * CPU registers saved during a context switch.
 */
typedef struct {
    unsigned long ra;
    unsigned long sp;

    unsigned long s0;
    unsigned long s1;
    unsigned long s2;
    unsigned long s3;
    unsigned long s4;
    unsigned long s5;
    unsigned long s6;
    unsigned long s7;
    unsigned long s8;
    unsigned long s9;
    unsigned long s10;
    unsigned long s11;
} context_t;

typedef struct {
    int pid;
    int parent_pid;
    process_state_t state;

    /*
     * Saved CPU context.
     */
    context_t context;

} process_t;

extern process_t process_table[MAX_PROCESSES];

/* Process management */
void process_init(void);
int process_fork(void);
void process_exit(int pid);
int process_wait(int parent_pid);

/* Scheduler */
void scheduler_init(void);
int scheduler_next(void);

#endif

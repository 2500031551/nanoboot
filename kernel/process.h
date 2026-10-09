
#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 4

typedef enum {
    UNUSED,
    READY,
    RUNNING,
    SLEEPING,
    EXITED
} process_state_t;

/*
 * Kernel CPU registers saved during a context switch.
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


/*
 * User CPU state saved when a trap occurs.
 */
typedef struct {
    unsigned long ra;
    unsigned long sp;

    unsigned long gp;
    unsigned long tp;

    unsigned long t0;
    unsigned long t1;
    unsigned long t2;

    unsigned long s0;
    unsigned long s1;

    unsigned long a0;
    unsigned long a1;
    unsigned long a2;
    unsigned long a3;
    unsigned long a4;
    unsigned long a5;
    unsigned long a6;
    unsigned long a7;

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

    unsigned long t3;
    unsigned long t4;
    unsigned long t5;
    unsigned long t6;

    unsigned long sepc;
    unsigned long sstatus;
} trapframe_t;


/*
 * Process Control Block.
 */
typedef struct {
    int pid;
    int parent_pid;
    process_state_t state;
    unsigned long wake_tick;

    context_t context;
    trapframe_t trapframe;
} process_t;

extern process_t process_table[MAX_PROCESSES];



/* Process management */
void test_context_switch(void);
void process_init(void);
int process_fork(void);
void process_exit(int pid);
int process_wait(int parent_pid);


/* Scheduler */
void scheduler_init(void);
int scheduler_next(void);

#endif

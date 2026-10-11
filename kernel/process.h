
#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 4
#define KERNEL_STACK_SIZE 4096

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


/* Must match the 288-byte frame layout in trap.S. */
typedef struct {
    unsigned long x[32];
    unsigned long sepc;
    unsigned long sstatus;
    unsigned long scause;
    unsigned long padding;
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

    /* Private supervisor stack used by this process's kernel context. */
    unsigned char kernel_stack[KERNEL_STACK_SIZE]
        __attribute__((aligned(16)));

    /* Private user stack; its page is mapped U=1 only for this process. */
    unsigned char user_stack[4096]
        __attribute__((aligned(4096)));
} process_t;

extern process_t process_table[MAX_PROCESSES];



/* Process management */
void test_context_switch(void);
void process_init(void);
int process_fork(void);
void process_exit(int pid);
int process_wait(int parent_pid);
int process_current_pid(void);
int process_parent_pid(int pid);
int process_sleep(int pid, unsigned long ticks);
int process_wakeup(unsigned long ticks);


/* Scheduler */
void scheduler_init(void);
int scheduler_next(void);

#endif

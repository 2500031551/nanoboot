
#include <stdint.h>
#include "process.h"

extern void shell_run(void);

#define SYS_GETPID 1
#define SYS_WRITE  2
#define SYS_EXIT   3
#define SYS_FORK   4
#define SYS_WAIT   5
#define SYS_SHELL  6
#define SYS_SLEEP  7

volatile uint8_t *uart = (uint8_t *)0x10000000UL;

void print_string(const char *msg)
{
    for (int i = 0; msg[i] != '\0'; i++) {
        *uart = msg[i];
    }
}

void print_number(uint64_t number)
{
    if (number == 0) {
        *uart = '0';
        return;
    }

    char digits[20];
    int i = 0;

    while (number > 0) {
        digits[i++] = '0' + (number % 10);
        number /= 10;
    }

    while (i > 0) {
        *uart = digits[--i];
    }
}

uint64_t syscall_handler(uint64_t syscall_number, uint64_t arg0)
{
    /* GETPID */
    if (syscall_number == SYS_GETPID) {
        print_string("getpid() called\n");

        int current_pid = process_current_pid();
        uint64_t pid = current_pid < 0 ? 0 : (uint64_t)current_pid;

        print_string("PID = ");
        print_number(pid);
        print_string("\n");

        return pid;
    }

    /* WRITE */
    if (syscall_number == SYS_WRITE) {
        print_string("write() called\n");
        print_string("Hello from user program!\n");
        return 1;
    }

    /* EXIT */
    if (syscall_number == SYS_EXIT) {
        print_string("exit() called\n");

        int current_pid = process_current_pid();
        if (current_pid < 0) {
            print_string("exit() failed: no running process\n");
            return (uint64_t)-1;
        }

        process_exit(current_pid);
        return 0;
    }

    /* FORK */
    if (syscall_number == SYS_FORK) {
        print_string("fork() called\n");

        int child_pid = process_fork();

        if (child_pid < 0) {
            print_string("fork() failed!\n");
            return (uint64_t)-1;
        }

        print_string("Parent PID = ");
        int parent_pid = process_current_pid();
        print_number(parent_pid < 0 ? 0 : (uint64_t)parent_pid);
        print_string("\n");

        print_string("Child PID = ");
        print_number((uint64_t)child_pid);
        print_string("\n");

        print_string("Child queued for round-robin scheduling\n");

        return (uint64_t)child_pid;
    }

    /* WAIT */
    if (syscall_number == SYS_WAIT) {
        print_string("wait() called\n");

        int parent_pid = process_current_pid();
        int child_pid = parent_pid < 0
            ? -1
            : process_wait(parent_pid);

        if (child_pid == -2) {
            print_string("Parent blocked until a child exits\n");
            return (uint64_t)-2;
        }
        if (child_pid < 0) {
            print_string("No exited child found!\n");
            return (uint64_t)-1;
        }

        print_string("Wait returned child PID = ");
        print_number((uint64_t)child_pid);
        print_string("\n");

        return (uint64_t)child_pid;
    }

    /* SLEEP: argument is a number of 10 ms scheduler ticks. */
    if (syscall_number == SYS_SLEEP) {
        int pid = process_current_pid();
        if (pid < 0 || process_sleep(pid, (unsigned long)arg0) < 0) {
            return (uint64_t)-1;
        }
        if (arg0 == 0) {
            return 0;
        }
        print_string("sleep() blocked until timer wakeup\n");
        return (uint64_t)-3; /* trap handler advances PC before blocking */
    }

    /* SHELL: enter the kernel shell after the user test completes. */
    if (syscall_number == SYS_SHELL) {
        print_string("Starting NanoBoot shell...\n");
        shell_run();
        return 0;
    }

    /* UNKNOWN SYSTEM CALL */
    print_string("Unknown system call!\n");
    return 0;
}

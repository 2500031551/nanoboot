#include <stdint.h>
#include "process.h"

#define SYS_GETPID 1
#define SYS_WRITE  2
#define SYS_EXIT   3
#define SYS_FORK   4
#define SYS_WAIT   5

volatile uint8_t *uart =
    (uint8_t *)0x10000000UL;


/*
 * Print a string to the UART.
 */
void print_string(const char *msg)
{
    for (int i = 0; msg[i] != '\0'; i++) {
        *uart = msg[i];
    }
}


/*
 * Print an unsigned number to the UART.
 */
void print_number(uint64_t number)
{
    if (number == 0) {
        *uart = '0';
        return;
    }

    char digits[20];
    int i = 0;

    while (number > 0) {
        digits[i++] =
            '0' + (number % 10);

        number /= 10;
    }

    while (i > 0) {
        *uart = digits[--i];
    }
}


/*
 * System call handler.
 */
uint64_t syscall_handler(uint64_t syscall_number)
{
    /*
     * -------------------------
     * GETPID
     * -------------------------
     */
    if (syscall_number == SYS_GETPID) {

        print_string(
            "getpid() called\n"
        );

        uint64_t pid = 1;

        print_string("PID = ");

        print_number(pid);

        print_string("\n");

        return pid;
    }


    /*
     * -------------------------
     * WRITE
     * -------------------------
     */
    if (syscall_number == SYS_WRITE) {

        print_string(
            "write() called\n"
        );

        const char *msg =
            "Hello from user program!\n";

        print_string(msg);

        return 1;
    }


    /*
     * -------------------------
     * EXIT
     * -------------------------
     */
    if (syscall_number == SYS_EXIT) {

        print_string(
            "exit() called\n"
        );

        process_exit(2);

        print_string(
            "Process finished!\n"
        );

        return 0;
    }


    /*
     * -------------------------
     * FORK
     * -------------------------
     */
    if (syscall_number == SYS_FORK) {

        print_string(
            "fork() called\n"
        );

        int child_pid =
            process_fork();

        if (child_pid < 0) {

            print_string(
                "fork() failed!\n"
            );

            return (uint64_t)-1;
        }

        print_string(
            "Parent PID = "
        );

        print_number(1);

        print_string("\n");


        print_string(
            "Child PID = "
        );

        print_number(
            (uint64_t)child_pid
        );

        print_string("\n");


        /*
         * Stage 7:
         *
         * The child process and its
         * CPU context have been created.
         *
         * We are NOT performing the
         * actual context switch yet.
         */
        print_string(
            "Context switch test ready\n"
        );

        return (uint64_t)child_pid;
    }


    /*
     * -------------------------
     * WAIT
     * -------------------------
     */
    if (syscall_number == SYS_WAIT) {

        print_string(
            "wait() called\n"
        );

        int child_pid =
            process_wait(1);

        if (child_pid < 0) {

            print_string(
                "No exited child found!\n"
            );

            return (uint64_t)-1;
        }

        print_string(
            "Wait returned child PID = "
        );

        print_number(
            (uint64_t)child_pid
        );

        print_string("\n");

        return (uint64_t)child_pid;
    }


    /*
     * -------------------------
     * UNKNOWN SYSTEM CALL
     * -------------------------
     */
    print_string(
        "Unknown system call!\n"
    );

    return 0;
}


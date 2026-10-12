
#include <stdint.h>
#include "process.h"
#include "ramfs.h"
#include "vm.h"

extern void shell_run(void);

#define SYS_GETPID 1
#define SYS_WRITE  2
#define SYS_EXIT   3
#define SYS_FORK   4
#define SYS_WAIT   5
#define SYS_SHELL  6
#define SYS_SLEEP  7
#define SYS_READ   8
#define SYS_COMMAND 9

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

extern uint64_t timer_tick_count(void);

static int user_stack_range(uint64_t address, uint64_t length)
{
    int pid = process_current_pid();
    if (pid < 1 || pid > MAX_PROCESSES || length > 256) return 0;
    process_t *p = &process_table[pid - 1];
    uint64_t low = (uint64_t)p->user_stack;
    uint64_t high = low + sizeof(p->user_stack);
    return address >= low && address <= high && length <= high - address;
}

static void uart_putc(char c)
{
    volatile uint8_t *lsr = (uint8_t *)0x10000005UL;
    volatile uint8_t *thr = (uint8_t *)0x10000000UL;
    while ((*lsr & 0x20) == 0) {}
    if (c == '\n') {
        *thr = '\r';
        while ((*lsr & 0x20) == 0) {}
    }
    *thr = (uint8_t)c;
}

static void kernel_text(const char *s) { while (*s) uart_putc(*s++); }

static void kernel_uint(uint64_t n)
{
    char digits[20]; int used = 0;
    if (!n) { uart_putc('0'); return; }
    while (n && used < 20) { digits[used++] = '0' + n % 10; n /= 10; }
    while (used) uart_putc(digits[--used]);
}

static int text_equal(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == 0 && *b == 0;
}

static void shell_command(const char *command)
{
    if (text_equal(command, "ps")) {
        kernel_text("PID PPID STATE\n");
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].state == UNUSED) continue;
            kernel_uint(process_table[i].pid); kernel_text(" ");
            kernel_uint(process_table[i].parent_pid); kernel_text(" ");
            switch (process_table[i].state) {
            case READY: kernel_text("READY\n"); break;
            case RUNNING: kernel_text("RUNNING\n"); break;
            case SLEEPING: kernel_text("SLEEPING\n"); break;
            case EXITED: kernel_text("EXITED\n"); break;
            default: kernel_text("UNUSED\n"); break;
            }
        }
    } else if (text_equal(command, "ticks") || text_equal(command, "uptime")) {
        uint64_t ticks = timer_tick_count();
        kernel_text("Timer ticks: "); kernel_uint(ticks);
        kernel_text(" (about "); kernel_uint(ticks / 100);
        kernel_text(" seconds)\n");
    } else if (text_equal(command, "uname")) {
        kernel_text("NanoBoot rv64 RISC-V Sv39\n");
    } else if (text_equal(command, "stage")) {
        kernel_text("Stage 9: RAM filesystem and shell commands\n");
        kernel_text("Try ls, cat about.txt, and isolation.\n");
    } else if (text_equal(command, "about")) {
        kernel_text("NanoBoot: RISC-V boot-to-shell teaching kernel.\n");
        kernel_text("U-mode syscalls, round-robin processes, Sv39, RAMFS.\n");
    } else if (text_equal(command, "isolation")) {
        int ok = 1;
        for (int owner = 0; owner < MAX_PROCESSES; owner++) {
            uint64_t own = (uint64_t)process_table[owner].user_stack;
            if (!vm_user_accessible(owner, own)) ok = 0;
            for (int other = 0; other < MAX_PROCESSES; other++) {
                if (other != owner &&
                    vm_user_accessible(owner,
                        (uint64_t)process_table[other].user_stack)) ok = 0;
            }
        }
        kernel_text(ok ? "User stack permission matrix: PASS\n"
                       : "User stack permission matrix: FAIL\n");
    } else if (text_equal(command, "ls")) {
        size_t count; const ramfs_file_t *files = ramfs_files(&count);
        for (size_t i = 0; i < count; i++) {
            kernel_text(files[i].name); kernel_text("\n");
        }
    } else if (command[0] == 'c' && command[1] == 'a' &&
               command[2] == 't' && command[3] == ' ') {
        const ramfs_file_t *file = ramfs_find(&command[4]);
        if (!file) { kernel_text("cat: file not found\n"); return; }
        for (size_t i = 0; i < file->size; i++) uart_putc(file->data[i]);
    } else {
        kernel_text("Unknown command. Type help.\n");
    }
}

uint64_t syscall_handler(uint64_t syscall_number, uint64_t arg0,
                         uint64_t arg1)
{
    /* GETPID */
    if (syscall_number == SYS_GETPID) {
        int current_pid = process_current_pid();
        uint64_t pid = current_pid < 0 ? 0 : (uint64_t)current_pid;
        return pid;
    }

    /* WRITE */
    if (syscall_number == SYS_WRITE) {
        if (!user_stack_range(arg0, arg1)) return (uint64_t)-1;
        const char *buffer = (const char *)arg0;
        for (uint64_t i = 0; i < arg1; i++) uart_putc(buffer[i]);
        return arg1;
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

    if (syscall_number == SYS_READ) {
        volatile uint8_t *lsr = (uint8_t *)0x10000005UL;
        volatile uint8_t *rhr = (uint8_t *)0x10000000UL;
        __asm__ volatile("csrsi sstatus, 2" : : : "memory");
        while ((*lsr & 1) == 0) {}
        uint64_t value = *rhr;
        __asm__ volatile("csrci sstatus, 2" : : : "memory");
        return value;
    }

    if (syscall_number == SYS_COMMAND) {
        if (!user_stack_range(arg0, arg1) || arg1 == 0 || arg1 > 63)
            return (uint64_t)-1;
        char command[64];
        const char *input = (const char *)arg0;
        for (uint64_t i = 0; i < arg1; i++) command[i] = input[i];
        command[arg1] = '\0';
        shell_command(command);
        return 0;
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

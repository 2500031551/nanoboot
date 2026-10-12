#include <stdint.h>
#include "process.h"
#include "ramfs.h"

#define UART_BASE 0x10000000UL
#define UART_RHR  (*(volatile uint8_t *)(UART_BASE + 0))
#define UART_THR  (*(volatile uint8_t *)(UART_BASE + 0))
#define UART_LSR  (*(volatile uint8_t *)(UART_BASE + 5))
#define UART_LSR_DR 0x01
#define UART_LSR_THRE 0x20

extern uint64_t timer_tick_count(void);
extern uint64_t timer_last_error(void);

static void putchar_uart(char c)
{
    while ((UART_LSR & UART_LSR_THRE) == 0) {
    }
    UART_THR = (uint8_t)c;
}

static void print(const char *s)
{
    while (*s) {
        if (*s == '\n') {
            putchar_uart('\r');
        }
        putchar_uart(*s++);
    }
}

static void print_number(uint64_t value)
{
    char digits[20];
    unsigned int count = 0;

    if (value == 0) {
        putchar_uart('0');
        return;
    }

    while (value != 0 && count < sizeof(digits)) {
        digits[count++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (count != 0) {
        putchar_uart(digits[--count]);
    }
}

static char getchar_uart(void)
{
    while ((UART_LSR & UART_LSR_DR) == 0) {
    }
    return (char)UART_RHR;
}

static int same(const char *a, const char *b)
{
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

void shell_run(void)
{
    char command[64];

    /* The shell runs inside the SYS_SHELL trap handler. Re-enable SIE so
     * timer ticks continue while the kernel waits for UART input. The trap
     * entry saves all registers, so a timer can safely interrupt this loop.
     */
    __asm__ volatile("csrsi sstatus, 2" : : : "memory");

    print("\nNanoBoot shell\nType help for commands.\n");

    for (;;) {
        print("nanoboot> ");

        int length = 0;

        for (;;) {
            char c = getchar_uart();

            if (c == '\r' || c == '\n') {
                print("\n");
                command[length] = '\0';
                break;
            }

            if ((c == '\b' || c == 127) && length > 0) {
                length--;
                print("\b \b");
                continue;
            }

            if (c >= 32 && c <= 126 && length < 63) {
                command[length++] = c;
                putchar_uart(c);
            }
        }

        if (same(command, "help")) {
            print("Commands: help, hello, ps, uptime, uname, ticks, ls, cat <file>, clear\n");
        } else if (same(command, "hello")) {
            print("Hello from NanoBoot!\n");
        } else if (same(command, "ps")) {
            print("PID  PPID  STATE\n");
            for (int i = 0; i < MAX_PROCESSES; i++) {
                const char *state;
                switch (process_table[i].state) {
                case READY: state = "READY"; break;
                case RUNNING: state = "RUNNING"; break;
                case SLEEPING: state = "SLEEPING"; break;
                case EXITED: state = "EXITED"; break;
                default: state = "UNUSED"; break;
                }
                if (process_table[i].state != UNUSED) {
                    print_number((uint64_t)process_table[i].pid);
                    print("   ");
                    print_number((uint64_t)process_table[i].parent_pid);
                    print("   ");
                    print(state);
                    print("\n");
                }
            }
        } else if (same(command, "uptime")) {
            uint64_t ticks = timer_tick_count();
            print("Uptime: ");
            print_number(ticks / 100);
            print(" seconds (");
            print_number(ticks);
            print(" timer ticks)\n");
        } else if (same(command, "uname")) {
            print("NanoBoot rv64 (RISC-V Sv39)\n");
        } else if (same(command, "ls")) {
            size_t count;
            const ramfs_file_t *files = ramfs_files(&count);
            for (size_t i = 0; i < count; i++) {
                print(files[i].name);
                print("\n");
            }
        } else if (command[0] == 'c' && command[1] == 'a' &&
                   command[2] == 't' && command[3] == ' ') {
            const ramfs_file_t *file = ramfs_find(&command[4]);
            if (file == 0) {
                print("cat: file not found\n");
            } else {
                for (size_t i = 0; i < file->size; i++) {
                    char c = file->data[i];
                    if (c == '\n') {
                        putchar_uart('\r');
                    }
                    putchar_uart(c);
                }
            }
        } else if (same(command, "ticks")) {
            uint64_t sstatus_value;
            uint64_t sie_value;
            __asm__ volatile("csrr %0, sstatus" : "=r"(sstatus_value));
            __asm__ volatile("csrr %0, sie" : "=r"(sie_value));
            print("Timer interrupts: ");
            print_number(timer_tick_count());
            print(" (SBI timer error: ");
            print_number(timer_last_error());
            print(", SIE: ");
            print_number((sstatus_value >> 1) & 1);
            print(", STIE: ");
            print_number((sie_value >> 5) & 1);
            print(")\n");
        } else if (same(command, "clear")) {
            print("\033[2J\033[H");
        } else if (command[0] == '\0') {
            /* Ignore an empty command. */
        } else {
            print("Unknown command. Type help.\n");
        }
    }
}

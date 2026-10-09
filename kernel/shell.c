#include <stdint.h>

#define UART_BASE 0x10000000UL
#define UART_RHR  (*(volatile uint8_t *)(UART_BASE + 0))
#define UART_THR  (*(volatile uint8_t *)(UART_BASE + 0))
#define UART_LSR  (*(volatile uint8_t *)(UART_BASE + 5))
#define UART_LSR_DR 0x01
#define UART_LSR_THRE 0x20

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
            print("Commands: help, hello, clear\n");
        } else if (same(command, "hello")) {
            print("Hello from NanoBoot!\n");
        } else if (same(command, "clear")) {
            print("\033[2J\033[H");
        } else if (command[0] == '\0') {
            /* Ignore an empty command. */
        } else {
            print("Unknown command. Type help.\n");
        }
    }
}


#include <stdint.h>

#define UART0 0x10000000UL
#define TIMEBASE_FREQ 10000000UL

static volatile uint8_t *uart = (uint8_t *)UART0;

static void print_text(const char *text)
{
    while (*text) {
        *uart = (uint8_t)*text++;
    }
}

static void print_number(uint64_t value)
{
    char digits[20];
    int count = 0;

    if (value == 0) {
        *uart = '0';
        return;
    }

    while (value > 0 && count < 20) {
        digits[count++] = '0' + (value % 10);
        value /= 10;
    }

    while (count > 0) {
        *uart = (uint8_t)digits[--count];
    }
}

uint64_t timer_now(void)
{
    uint64_t ticks;

    __asm__ volatile("rdtime %0" : "=r"(ticks));

    return ticks;
}

void timer_test(void)
{
    uint64_t ticks = timer_now();

    print_text("Timer frequency: ");
    print_number(TIMEBASE_FREQ);
    print_text(" Hz\n");

    print_text("Current timer ticks: ");
    print_number(ticks);
    print_text("\n");
}
void timer_delay_ms(uint64_t milliseconds)
{
    uint64_t start = timer_now();
    uint64_t ticks_to_wait =
        (TIMEBASE_FREQ / 1000UL) * milliseconds;

    while ((timer_now() - start) < ticks_to_wait) {
        __asm__ volatile("nop");
    }
}


#include <stdint.h>

#define UART0 0x10000000UL
#define TIMEBASE_FREQ 10000000UL
#define TIMER_INTERVAL_TICKS (TIMEBASE_FREQ / 100UL)

static volatile uint8_t *uart = (uint8_t *)UART0;
static volatile uint64_t timer_interrupts;
static volatile uint64_t timer_sbi_error;
uint64_t timer_now(void);

static void sbi_set_timer(uint64_t deadline)
{
    register uint64_t a0 asm("a0") = deadline;
    register uint64_t a1 asm("a1") = 0;
    register uint64_t a6 asm("a6") = 0;
    register uint64_t a7 asm("a7") = 0x54494D45UL; /* SBI TIME extension */
    __asm__ volatile("ecall"
                     : "+r"(a0), "+r"(a1)
                     : "r"(a6), "r"(a7)
                     : "memory");
    timer_sbi_error = a0;
}

void timer_init(void)
{
    timer_interrupts = 0;
    sbi_set_timer(timer_now() + TIMER_INTERVAL_TICKS);

    /* Enable supervisor timer interrupts; global enable is set by sret. */
    uint64_t timer_enable = 1UL << 5;
    __asm__ volatile("csrs sie, %0" : : "r"(timer_enable));
}

void timer_on_interrupt(void)
{
    timer_interrupts++;
    sbi_set_timer(timer_now() + TIMER_INTERVAL_TICKS);
}

uint64_t timer_tick_count(void)
{
    return timer_interrupts;
}

uint64_t timer_last_error(void)
{
    return timer_sbi_error;
}

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

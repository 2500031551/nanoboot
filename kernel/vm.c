#include "vm.h"

#define SATP_MODE_SV39 (8UL << 60)

/*
 * Sv39 page tables.
 *
 * Each page table = 4096 bytes = 512 PTEs.
 */
static pagetable_t root_table
    __attribute__((aligned(PAGE_SIZE)));

static pagetable_t kernel_level1
    __attribute__((aligned(PAGE_SIZE)));

static pagetable_t kernel_level0
    __attribute__((aligned(PAGE_SIZE)));

static pagetable_t uart_level1
    __attribute__((aligned(PAGE_SIZE)));

static pagetable_t uart_level0
    __attribute__((aligned(PAGE_SIZE)));

extern void user_program(void);

#define PA_TO_PPN(pa) ((pa) >> 12)

pte_t make_pte(uint64_t physical_address, uint64_t flags)
{
    return (PA_TO_PPN(physical_address) << 10)
           | flags
           | PTE_V;
}

static void clear_table(pagetable_t table)
{
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        table[i] = 0;
    }
}

void vm_init(void)
{
    clear_table(root_table);
    clear_table(kernel_level1);
    clear_table(kernel_level0);
    clear_table(uart_level1);
    clear_table(uart_level0);
}

/*
 * Map one kernel page.
 *
 * Kernel addresses:
 * 0x80200000 - 0x803FFFFF
 */
static void map_kernel_page(
    uint64_t virtual_address,
    uint64_t physical_address,
    uint64_t flags)
{
    uint64_t vpn2 =
        (virtual_address >> 30) & 0x1FF;

    uint64_t vpn1 =
        (virtual_address >> 21) & 0x1FF;

    uint64_t vpn0 =
        (virtual_address >> 12) & 0x1FF;

    /*
     * Root -> kernel level 1.
     */
    root_table[vpn2] =
        make_pte(
            (uint64_t)kernel_level1,
            0
        );

    /*
     * Kernel level 1 -> level 0.
     */
    kernel_level1[vpn1] =
        make_pte(
            (uint64_t)kernel_level0,
            0
        );

    /*
     * Level 0 -> physical page.
     */
    kernel_level0[vpn0] =
        make_pte(
            physical_address,
            flags
        );
}

/*
 * Map UART page.
 */
static void map_uart(void)
{
    uint64_t virtual_address = 0x10000000UL;
    uint64_t physical_address = 0x10000000UL;

    uint64_t vpn2 =
        (virtual_address >> 30) & 0x1FF;

    uint64_t vpn1 =
        (virtual_address >> 21) & 0x1FF;

    uint64_t vpn0 =
        (virtual_address >> 12) & 0x1FF;

    /*
     * Root -> UART level 1.
     */
    root_table[vpn2] =
        make_pte(
            (uint64_t)uart_level1,
            0
        );

    /*
     * UART level 1 -> level 0.
     */
    uart_level1[vpn1] =
        make_pte(
            (uint64_t)uart_level0,
            0
        );

    /*
     * UART physical page.
     */
    uart_level0[vpn0] =
        make_pte(
            physical_address,
            PTE_R | PTE_W
        );
}

void map_page(
    pagetable_t table,
    uint64_t virtual_address,
    uint64_t physical_address,
    uint64_t flags)
{
    /*
     * This function is kept for the VM interface.
     *
     * Stage 6 uses the explicit mappings above.
     */
    (void)table;
    (void)virtual_address;
    (void)physical_address;
    (void)flags;
}

uint64_t get_physical_address(pte_t entry)
{
    return ((entry >> 10) << 12);
}

void vm_kernel_map(void)
{
    /*
     * Map the complete NanoBoot kernel region.
     */
    for (
        uint64_t address = 0x80200000UL;
        address < 0x80400000UL;
        address += PAGE_SIZE
    ) {
        map_kernel_page(
            address,
            address,
            PTE_R | PTE_W | PTE_X
        );
    }

    /*
     * Map UART.
     */
    map_uart();

    /*
     * Give user mode execute permission
     * on the page containing user_program().
     */
    uint64_t user_address =
        (uint64_t)user_program;

    uint64_t user_page =
        user_address & ~(PAGE_SIZE - 1);

    uint64_t vpn2 =
        (user_page >> 30) & 0x1FF;

    uint64_t vpn1 =
        (user_page >> 21) & 0x1FF;

    uint64_t vpn0 =
        (user_page >> 12) & 0x1FF;

    root_table[vpn2] =
        make_pte(
            (uint64_t)kernel_level1,
            0
        );

    kernel_level1[vpn1] =
        make_pte(
            (uint64_t)kernel_level0,
            0
        );

    kernel_level0[vpn0] =
        make_pte(
            user_page,
            PTE_R | PTE_X | PTE_U
        );
}

void vm_enable(void)
{
    volatile uint8_t *uart =
        (uint8_t *)0x10000000UL;

    const char *msg1 =
        "vm_enable: calculating SATP\n";

    for (int i = 0; msg1[i] != '\0'; i++) {
        *uart = msg1[i];
    }

    uint64_t root_address =
        (uint64_t)root_table;

    uint64_t satp_value =
        SATP_MODE_SV39 |
        (root_address >> 12);

    const char *msg2 =
        "vm_enable: SATP value ready\n";

    for (int i = 0; msg2[i] != '\0'; i++) {
        *uart = msg2[i];
    }

    /*
     * Print root page-table address.
     */
    const char hex[] =
        "0123456789ABCDEF";

    const char *msg3 =
        "Root table = 0x";

    for (int i = 0; msg3[i] != '\0'; i++) {
        *uart = msg3[i];
    }

    for (int shift = 60; shift >= 0; shift -= 4) {
        *uart = hex[(root_address >> shift) & 0xF];
    }

    *uart = '\n';

    /*
     * Print SATP value.
     */
    const char *msg4 =
        "SATP value = 0x";

    for (int i = 0; msg4[i] != '\0'; i++) {
        *uart = msg4[i];
    }

    for (int shift = 60; shift >= 0; shift -= 4) {
        *uart = hex[(satp_value >> shift) & 0xF];
    }

    *uart = '\n';

    /*
     * Do NOT write SATP yet.
     */
    const char *msg5 =
        "SATP diagnostic complete\n";

    for (int i = 0; msg5[i] != '\0'; i++) {
        *uart = msg5[i];
    }
}

pagetable_t *get_kernel_pagetable(void)
{
    return &root_table;
}

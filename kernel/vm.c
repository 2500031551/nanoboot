
#include "vm.h"

#define SATP_MODE_SV39 (8UL << 60)
#define PTE_PPN_MASK ((1UL << 44) - 1)

/* Static page tables, each aligned to one page. */
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
extern void child_program(void);

pte_t make_pte(uint64_t physical_address, uint64_t flags)
{
    return ((physical_address >> 12) << 10)
           | flags | PTE_V;
}

uint64_t get_physical_address(pte_t entry)
{
    return ((entry >> 10) & PTE_PPN_MASK) << 12;
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
 * Map pages only in the two supported regions:
 * kernel/user region at 0x80200000 and UART at 0x10000000.
 */
void map_page(
    pagetable_t table,
    uint64_t virtual_address,
    uint64_t physical_address,
    uint64_t flags)
{
    uint64_t vpn2 = (virtual_address >> 30) & 0x1FF;
    uint64_t vpn1 = (virtual_address >> 21) & 0x1FF;
    uint64_t vpn0 = (virtual_address >> 12) & 0x1FF;

    pagetable_t *level1;
    pagetable_t *level0;

    if (table != root_table) {
        return;
    }

    if (vpn2 == 2) {
        level1 = &kernel_level1;
        level0 = &kernel_level0;
    } else if (vpn2 == 0) {
        level1 = &uart_level1;
        level0 = &uart_level0;
    } else {
        return;
    }

    /* Create or verify the root-to-level-1 link. */
    pte_t root_entry = table[vpn2];
    pte_t expected_root = make_pte((uint64_t)*level1, 0);

    if (root_entry & PTE_V) {
        if (get_physical_address(root_entry) !=
            get_physical_address(expected_root)) {
            return;
        }
    } else {
        table[vpn2] = expected_root;
    }

    /* Create or verify the level-1-to-level-0 link. */
    pte_t middle_entry = (*level1)[vpn1];
    pte_t expected_middle = make_pte((uint64_t)*level0, 0);

    if (middle_entry & PTE_V) {
        if (get_physical_address(middle_entry) !=
            get_physical_address(expected_middle)) {
            return;
        }
    } else {
        (*level1)[vpn1] = expected_middle;
    }

    /* Install the leaf mapping. */
    (*level0)[vpn0] = make_pte(physical_address, flags);
}

void vm_kernel_map(void)
{
    /* Identity-map kernel memory and static page tables. */
    for (uint64_t address = 0x80200000UL;
         address < 0x80400000UL;
         address += PAGE_SIZE) {
        map_page(root_table, address, address,
                 PTE_R | PTE_W | PTE_X);
    }

    /* Identity-map the UART device page. */
    map_page(root_table, 0x10000000UL, 0x10000000UL,
             PTE_R | PTE_W);

    /* Map both user program pages as user-readable/executable. */
    uint64_t user_page =
        ((uint64_t)user_program) & ~(PAGE_SIZE - 1);

    uint64_t child_page =
        ((uint64_t)child_program) & ~(PAGE_SIZE - 1);

    map_page(root_table, user_page, user_page,
             PTE_R | PTE_X | PTE_U);

    map_page(root_table, child_page, child_page,
             PTE_R | PTE_X | PTE_U);
}

void vm_enable(void)
{
    uint64_t satp_value =
        SATP_MODE_SV39 | ((uint64_t)root_table >> 12);

    asm volatile(
        "csrw satp, %0\n"
        "sfence.vma zero, zero\n"
        :
        : "r"(satp_value)
        : "memory"
    );
}

pagetable_t *get_kernel_pagetable(void)
{
    return &root_table;
}

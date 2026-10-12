#include "vm.h"
#include "process.h"

#define SATP_MODE_SV39 (8UL << 60)
#define PTE_PPN_MASK ((1UL << 44) - 1)

/* Each process gets a private root and private lower-level kernel-region
 * tables. UART page-table levels are shared because the device is kernel-only.
 */
static pagetable_t process_roots[MAX_PROCESSES]
    __attribute__((aligned(PAGE_SIZE)));
static pagetable_t kernel_level1[MAX_PROCESSES]
    __attribute__((aligned(PAGE_SIZE)));
static pagetable_t kernel_level0[MAX_PROCESSES]
    __attribute__((aligned(PAGE_SIZE)));
static pagetable_t uart_level1
    __attribute__((aligned(PAGE_SIZE)));
static pagetable_t uart_level0
    __attribute__((aligned(PAGE_SIZE)));

extern char __usertext_start[];
extern char __usertext_end[];
extern char __userrodata_start[];
extern char __userrodata_end[];

pte_t make_pte(uint64_t physical_address, uint64_t flags)
{
    return ((physical_address >> 12) << 10) | flags | PTE_V;
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
    for (int i = 0; i < MAX_PROCESSES; i++) {
        clear_table(process_roots[i]);
        clear_table(kernel_level1[i]);
        clear_table(kernel_level0[i]);
    }
    clear_table(uart_level1);
    clear_table(uart_level0);
}

void map_page(
    pagetable_t table,
    uint64_t virtual_address,
    uint64_t physical_address,
    uint64_t flags)
{
    int process_index = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (table == process_roots[i]) {
            process_index = i;
            break;
        }
    }
    if (process_index < 0) {
        return;
    }

    uint64_t vpn2 = (virtual_address >> 30) & 0x1FF;
    uint64_t vpn1 = (virtual_address >> 21) & 0x1FF;
    uint64_t vpn0 = (virtual_address >> 12) & 0x1FF;
    pte_t *level1;
    pte_t *level0;

    if (vpn2 == 2) {
        level1 = kernel_level1[process_index];
        level0 = kernel_level0[process_index];
    } else if (vpn2 == 0) {
        level1 = uart_level1;
        level0 = uart_level0;
    } else {
        return;
    }

    pte_t expected_root = make_pte((uint64_t)level1, 0);
    if (table[vpn2] & PTE_V) {
        if (get_physical_address(table[vpn2]) !=
            get_physical_address(expected_root)) {
            return;
        }
    } else {
        table[vpn2] = expected_root;
    }

    pte_t expected_middle = make_pte((uint64_t)level0, 0);
    if (level1[vpn1] & PTE_V) {
        if (get_physical_address(level1[vpn1]) !=
            get_physical_address(expected_middle)) {
            return;
        }
    } else {
        level1[vpn1] = expected_middle;
    }

    level0[vpn0] = make_pte(physical_address, flags);
}

void vm_kernel_map(void)
{
    for (int process = 0; process < MAX_PROCESSES; process++) {
        /* Kernel and page-table memory: identity mapped supervisor-only. */
        for (uint64_t address = 0x80200000UL;
             address < 0x80400000UL;
             address += PAGE_SIZE) {
            map_page(process_roots[process], address, address,
                     PTE_R | PTE_W | PTE_X);
        }

        map_page(process_roots[process], 0x10000000UL, 0x10000000UL,
                 PTE_R | PTE_W);

        for (uint64_t address = (uint64_t)__usertext_start & ~(PAGE_SIZE - 1UL);
             address < (uint64_t)__usertext_end; address += PAGE_SIZE) {
            map_page(process_roots[process], address, address,
                     PTE_R | PTE_X | PTE_U);
        }
        for (uint64_t address = (uint64_t)__userrodata_start & ~(PAGE_SIZE - 1UL);
             address < (uint64_t)__userrodata_end; address += PAGE_SIZE) {
            map_page(process_roots[process], address, address,
                     PTE_R | PTE_U);
        }

        /* A process can access only its own user stack. Other stacks retain
         * supervisor-only mappings so trap frames remain reachable in S mode
         * when a different process page table is activated mid-trap.
         */
        for (int owner = 0; owner < MAX_PROCESSES; owner++) {
            uint64_t stack_page =
                (uint64_t)process_table[owner].user_stack;
            uint64_t stack_flags = PTE_R | PTE_W;
            if (owner == process) {
                stack_flags |= PTE_U;
            }
            map_page(process_roots[process], stack_page, stack_page,
                     stack_flags);
        }
    }
}

void vm_switch_process(int process_index)
{
    if (process_index < 0 || process_index >= MAX_PROCESSES) {
        return;
    }
    uint64_t satp_value = SATP_MODE_SV39 |
                          ((uint64_t)process_roots[process_index] >> 12);
    __asm__ volatile("csrw satp, %0\n"
                     "sfence.vma zero, zero\n"
                     : : "r"(satp_value) : "memory");
}

int vm_user_accessible(int process_index, uint64_t virtual_address)
{
    if (process_index < 0 || process_index >= MAX_PROCESSES) return 0;
    uint64_t vpn2 = (virtual_address >> 30) & 0x1ff;
    uint64_t vpn1 = (virtual_address >> 21) & 0x1ff;
    uint64_t vpn0 = (virtual_address >> 12) & 0x1ff;
    pte_t root = process_roots[process_index][vpn2];
    if (!(root & PTE_V)) return 0;
    pagetable_t *level1 = (pagetable_t *)get_physical_address(root);
    pte_t middle = (*level1)[vpn1];
    if (!(middle & PTE_V)) return 0;
    pagetable_t *level0 = (pagetable_t *)get_physical_address(middle);
    pte_t leaf = (*level0)[vpn0];
    return (leaf & PTE_V) && (leaf & PTE_U);
}

void vm_enable(void)
{
    vm_switch_process(0);
    /* SUM lets supervisor trap code access user register frames/stacks. */
    uint64_t sum = 1UL << 18;
    __asm__ volatile("csrs sstatus, %0" : : "r"(sum) : "memory");
}

pagetable_t *get_kernel_pagetable(void)
{
    return &process_roots[0];
}

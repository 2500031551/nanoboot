#ifndef VM_H
#define VM_H

#include <stdint.h>

/*
 * Sv39 uses 4 KiB pages.
 */
#define PAGE_SIZE 4096

/*
 * Each page table contains 512 entries.
 */
#define PAGE_ENTRIES 512

/*
 * RISC-V Page Table Entry flags.
 */
#define PTE_V  (1UL << 0)
#define PTE_R  (1UL << 1)
#define PTE_W  (1UL << 2)
#define PTE_X  (1UL << 3)
#define PTE_U  (1UL << 4)

/*
 * One page-table entry is 64 bits.
 */
typedef uint64_t pte_t;

/*
 * One Sv39 page table contains 512 entries.
 */
typedef pte_t pagetable_t[PAGE_ENTRIES];

/* Page-table functions */
pte_t make_pte(uint64_t physical_address, uint64_t flags);

void map_page(
    pagetable_t table,
    uint64_t virtual_address,
    uint64_t physical_address,
    uint64_t flags
);

uint64_t get_physical_address(pte_t entry);

void vm_init(void);

pagetable_t *get_kernel_pagetable(void);
void vm_kernel_map(void);
void vm_enable(void);
#endif

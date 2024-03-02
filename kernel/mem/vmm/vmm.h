#ifndef VMM_H
#define VMM_H

#include <stdint.h>

extern struct page_table* kernel_pml4;

#define PML4_IDX(virt) (((virt) >> 39) & 0x1ff)
#define PDP_IDX(virt)  (((virt) >> 30) & 0x1ff)
#define PD_IDX(virt)   (((virt) >> 21) & 0x1ff)
#define PT_IDX(virt)   (((virt) >> 12) & 0x1ff)
// #define POFF_IDX(virt) (((virt)        & 0xfff)

#define PTE_ADDR_MASK (0x000ffffffffff000)
#define PTE_GET_ADDR(VALUE) ((VALUE) & PTE_ADDR_MASK)
#define PTE_GET_FLAGS(VALUE) ((VALUE) & ~PTE_ADDR_MASK)

#define PTE_PRESENT (1ull << 0ull)
#define PTE_WRITABLE (1ull << 1ull)
#define PTE_USER (1ull << 2ull)

typedef uint64_t page_table_entry;
// typedef uint64_t* PT;
// typedef uint64_t* PD;
// typedef uint64_t* PDP;
// typedef uint64_t* PML4;

struct page_table
{
    uint64_t entries[512];
} __attribute__((packed));

uint64_t vmm_map_page(struct page_table* pml4, const uint64_t phys, const uintptr_t virt);
void vmm_init();
uint8_t dump_pte(const uint64_t virt);

#endif /* VMM_H */

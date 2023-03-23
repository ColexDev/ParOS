#include <stdint.h>

#include "../stdlib/util.h"
#include "paging.h"
#include "pmm.h"

uint32_t kernel_pdir[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));
uint32_t first_page_table[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));

extern uint32_t used_memory;

extern void write_cr3(uint32_t*);
extern void write_cr0(uint32_t);
extern uint32_t read_cr0();

void
map_page(void* phys, void* virt)
{
}

void
alloc_page(uint32_t* page, uint8_t is_kernel, uint8_t is_writeable)
{
    if (GET_PAGE_FRAME(*page) != 0) /* Already alloced */
        return;

    uint32_t frame = pmm_find_free_frame();
    pmm_set_frame(frame);

    used_memory += PAGE_FRAME_SIZE;

    *page |= PRESENT;
    *page |= is_writeable;
    *page |= !is_kernel;
    *page |= frame;
}

void
free_page(uint32_t* page)
{
    if (!(GET_PAGE_FRAME(*page)))
        return;

    pmm_clear_frame(GET_PAGE_FRAME(*page));

    /* Clears page frame, 0xFFFFF is the most significant 20 bits */
    *page &= ~0xFFFFF;
    *page &= ~PRESENT;
}

void 
enable_paging()
{
   asm volatile("mov %0, %%cr3":: "r"(kernel_pdir));
   uint32_t cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000;
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void
init_paging()
{
    for(int i = 0; i < 1024; i++) {
        kernel_pdir[i] = 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
    }

    /* Identity maps the first 4 MiB of memory */
    for (int i = 0; i < 1024; i++) {
        first_page_table[i] = (i * 0x1000) | 3;
    }

    kernel_pdir[0] = ((uint32_t)first_page_table) | 3;
    enable_paging();
}

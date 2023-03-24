#include <stdint.h>

#include "../stdlib/util.h"
#include "paging.h"
#include "pmm.h"
#include "../stdlib/util.h"
#include "../drivers/tty.h"

uint32_t kernel_pdir[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));
uint32_t first_page_table[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));

extern uint32_t used_memory;

extern void write_cr3(uint32_t*);
extern void write_cr0(uint32_t);
extern uint32_t read_cr0();

/* HAVE TO MAP PAGE ALIGNED ADDRESSES */
void
map_page(void* phys, void* virt)
{
}

uint32_t
get_page(uint32_t virt)
{
    uint32_t* table = (uint32_t*)kernel_pdir[PAGE_DIRECTORY_INDEX(virt)];
    uint32_t page = table[PAGE_TABLE_INDEX(virt)];
    return page;
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
    *page |= is_writeable << 1;
    *page |= !is_kernel << 2;
    *page |= frame << 12;
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
   uint32_t cr0 = 0;

   asm volatile("mov %0, %%cr3":: "r"(kernel_pdir));
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000;
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void
init_paging()
{
    /* Sets all page tables to not present */
    for(int i = 0; i < 1024; i++) {
        kernel_pdir[i] = 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
    }

    /* Identity maps the first 4 MiB of memory */
    uint32_t page = 0;
    for (int i = 0; i < 1024; i++) {
        page = 0; /* overwrite past flags */
        alloc_page(&page, 1, 1);
        first_page_table[i] = page;
    }

    /* Sets first page table in the page directory */
    kernel_pdir[0] = (uint32_t)first_page_table | 3;
    enable_paging();

    char buf[128];
    itoa(get_page(0x1000 * 3), buf, 10);
    puts(buf);
    puts("\n");
}

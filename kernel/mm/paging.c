#include <stdint.h>

#include "../stdlib/util.h"
#include "paging.h"
#include "pmm.h"

// struct page_directory kernel_pdir __attribute__((aligned(PAGE_FRAME_SIZE)));
uint32_t* kernel_pdir[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));
uint32_t page_table[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));

extern uint32_t used_memory;

void
map_page(void* phys, void* virt)
{
    page_table[0]  = 0 | 3;
    kernel_pdir[0] = page_table;
}

void
alloc_page(struct page* p, uint8_t is_kernel, uint8_t is_writeable)
{
    if (p->frame != 0) /* Already alloced */
        return;

    uint32_t frame = pmm_find_free_frame();
    pmm_set_frame(frame);

    used_memory += PAGE_FRAME_SIZE;

    p->present = PRESENT;
    p->rw      = is_writeable;
    p->user    = !is_kernel;
    p->frame   = frame;
}

void
free_page(struct page* p)
{
    if (!p->frame)
        return;

    pmm_clear_frame(p->frame);

    p->frame   = 0;
    p->present = 0;
}

struct page*
get_page(uint32_t address, struct page_directory* dir)
{
    uint32_t frame = address / PAGE_FRAME_SIZE;
    uint32_t table_index = frame / PAGE_FRAME_SIZE;

    if (dir->tables[table_index])
        return &dir->tables[table_index]->pages[frame % PAGE_FRAME_SIZE];
    /* else */

    /* Create the table if it does not exist */
    dir->tables[table_index] = (struct page_table*)pmm_alloc_frame();
    memset(dir->tables[table_index], 0, PAGE_FRAME_SIZE);
    return &dir->tables[table_index]->pages[frame % PAGE_FRAME_SIZE];
}

void
init_paging()
{
    // struct page p;
    // pmm_alloc_page(&p, 1, 1);
    for(int i = 1; i < 1024; i++)
    {
        kernel_pdir[i] = 0 | 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
    }
}

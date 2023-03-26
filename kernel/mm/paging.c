#include <stdint.h>

#include "paging.h"
#include "pmm.h"
#include "../stdlib/util.h"
#include "../stdlib/util.h"
#include "../drivers/tty.h"

uint32_t kernel_pdir[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));
uint32_t first_page_table[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));

extern uint32_t used_memory;

extern void write_cr3(uint32_t*);
extern void write_cr0(uint32_t);
extern uint32_t read_cr0();

/* HAVE TO MAP PAGE ALIGNED ADDRESSES */
uint32_t
map_page(uint32_t virt, uint8_t is_kernel, uint8_t is_writeable)
{
    uint32_t page;
    uint32_t table = kernel_pdir[PAGE_DIRECTORY_INDEX(virt)];

    // if (get_page(virt, 1) != 0)
    //     return 0;

    /* Table does not exist */
    if (!(table & PAGE_PRESENT)) {
        /* Create table */
    } else {
        /* Remove control bits, this gets the tables address */
        table &= PAGE_TABLE_ADDRESS_MASK;
    }

    /* Allocate the page */
    alloc_page(&page, is_kernel, is_writeable);

    /* Put the page in the table */
    ((uint32_t*)table)[PAGE_TABLE_INDEX(virt)] = page;

    /* Flush the TLB entry for the virtual address */
    asm volatile("invlpg (%0)" ::"r" (virt) : "memory");
}

void
create_page_table()
{
}

/* This needs to return 0 if not present, or some special value */
uint32_t
get_page(uint32_t virt, uint8_t create)
{
    uint32_t page = 0;
    /* NOTE: THIS WILL BE 2 IF THE PAGE TABLE DOES NOT EXIST */
    uint32_t table = kernel_pdir[PAGE_DIRECTORY_INDEX(virt)];

    /* NOTE: I think I should only create tables in map_page() */
    // if ((table == 2) && create) {
    //     /* create table */
    //     table = pmm_find_free_frame();
    //     memset((void*)table, 0, PAGE_FRAME_SIZE);
    //
    //     kernel_pdir[PAGE_DIRECTORY_INDEX(virt)] = table | 3;
    // } else {
    //     table &= ~0xFFF;
    // }

    table &= PAGE_TABLE_ADDRESS_MASK;
    /* Remove control bits, this gets the tables address */

    page = ((uint32_t*)table)[PAGE_TABLE_INDEX(virt)];

    /* NOTE: Change this to check if the page is present or not once I fix the table code */
    /* If page is garbage (does not exist), return 0 */
    if ((page >> 12) > 1023)
        return 0;
    else
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

    /* Set page table entry flags */
    *page |= PAGE_PRESENT;
    *page |= is_writeable << PAGE_WRITABLE_BIT;
    *page |= !is_kernel << PAGE_USER_BIT;
    *page |= frame << PAGE_FRAME_SHIFT;
}

void
free_page(uint32_t* page)
{
    if (!(GET_PAGE_FRAME(*page)))
        return;

    pmm_clear_frame(GET_PAGE_FRAME(*page));

    /* Clears page frame, 0xFFFFF is the most significant 20 bits */
    *page &= ~0xFFFFF;
    *page &= ~PAGE_PRESENT;
}

void 
enable_paging()
{
    uint32_t cr0 = 0;

    /* Load page directory base address into CR3 */
    asm volatile("mov %0, %%cr3":: "r"(kernel_pdir));

    /* Enable paging by setting the paging bit in CR0 */
    asm volatile("mov %%cr0, %0; \
                  orl %1, %0; \
                  mov %0, %%cr0" 
                  : "=r"(cr0) 
                  : "i"((1 << CR0_PG_BIT)));
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

    // char buf[128];
    // itoa(get_page(0x1000 * 1023, 1), buf, 16);
    // puts("Page: 0x");
    // puts(buf);
    // puts("\n");
}

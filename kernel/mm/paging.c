#include <stdint.h>

#include "paging.h"
#include "pmm.h"
#include "../stdlib/util.h"
#include "../drivers/tty.h"

static uint32_t kernel_pdir[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));
static uint32_t first_page_table[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));

static uint8_t paging_enabled = 0;

void
map_page(uint32_t virt, uint32_t phys)
{
    uint32_t* page_directory;
    uint32_t page_table;

    if (paging_enabled)
        page_directory = (uint32_t*)0xFFFFF000;
    else
        page_directory = kernel_pdir;

    if (!(page_directory[PAGE_DIRECTORY_INDEX(virt)] & 1)) {
        create_page_table(virt);
    }

    if (phys == 0) {
        phys = pmm_alloc_frame();
    }

    if (paging_enabled)
        page_table = 0xFFC00000 + ((virt >> 12) * 4);
    else 
        page_table = (page_directory[PAGE_DIRECTORY_INDEX(virt)] & PAGE_TABLE_ADDRESS_MASK);

    /* NOTE: PHYS is basically not used now, CANNOT choose a physical PAGE_TABLE_ADDRESS_MASK
     * unless this is setup in an else under if (phys == 0) */
    // pmm_set_frame(phys / 0x1000);

    if (paging_enabled)
        *((uint32_t*)page_table) = phys | 3;
    else
        ((uint32_t*)page_table)[PAGE_TABLE_INDEX(virt)] = phys | 3;

    /* Flush the TLB entry for the virtual address */
    asm volatile("invlpg (%0)" ::"r" (virt) : "memory");
}

void
create_page_table(uint32_t virt)
{
    uint32_t* page_directory;
    uint32_t new_table = pmm_alloc_frame();

    if (paging_enabled)
        page_directory = (uint32_t*)0xFFFFF000;
    else
        page_directory = kernel_pdir;

    page_directory[PAGE_DIRECTORY_INDEX(virt)] = (new_table * 0x1000) | 3;

    for (int i = 0; i < 1024; i++) {
        ((uint32_t*)new_table)[i] = 0;
    }
}

/* This needs to return 0 if not present, or some special value */
uint32_t
get_page(uint32_t virt)
{
    uint32_t page = 0;
    uint32_t* page_directory;
    uint32_t page_table;

    if (paging_enabled)
        page_directory = (uint32_t*)0xFFFFF000;
    else
        page_directory = kernel_pdir;

    if (paging_enabled)
        page_table = 0xFFC00000 + ((virt >> 12) * 4);
    else 
        page_table = (page_directory[PAGE_DIRECTORY_INDEX(virt)] & PAGE_TABLE_ADDRESS_MASK);

    /* Remove control bits, this gets the tables address */
    page_table &= PAGE_TABLE_ADDRESS_MASK;

    // if (paging_enabled)
        // page = *((uint32_t*)page_table);
    // else
    /* I think I should always do this one, the above just gets 
     * the first entry in the page table */
    page = ((uint32_t*)page_table)[PAGE_TABLE_INDEX(virt)];

    /* Remove control bits, this gets the pages address */
    page &= PAGE_TABLE_ADDRESS_MASK;

    /* NOTE: Change this to check if the page is present or not once I fix the table code */
    /* If page is garbage (does not exist), return 0 */
    // if ((page >> 12) > 1023)
    //     return 0;
    // else
    return page;
}

// void
// alloc_page(uint32_t* page, uint8_t is_kernel, uint8_t is_writeable)
// {
//     uint32_t frame;
//     if (GET_PAGE_FRAME(*page) != 0) /* Already alloced */
//         return;
//
//     /* TODO: There should be a function for this in the pmm */
//     frame = pmm_find_free_frame();
//
//     pmm_set_frame(frame);
//
//     used_memory += PAGE_FRAME_SIZE;
//
//     /* Set page table entry flags */
//     *page |= PAGE_PRESENT;
//     *page |= is_writeable << PAGE_WRITABLE_BIT;
//     *page |= !is_kernel << PAGE_USER_BIT;
//     *page |= frame << PAGE_FRAME_SHIFT;
// }

// void
// free_page(uint32_t* page)
// {
//     if (!(GET_PAGE_FRAME(*page)))
//         return;
//
//     pmm_clear_frame(GET_PAGE_FRAME(*page));
//
//     /* Clears page frame, 0xFFFFF is the most significant 20 bits */
//     *page &= ~0xFFFFF;
//     *page &= ~PAGE_PRESENT;
// }

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
    paging_enabled = 1;
}

void
init_paging()
{
    /* Sets all page tables to not present */
    for(uint16_t i = 0; i < 1024; i++) {
        kernel_pdir[i] = 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
    }
    /* Setup recursive paging */
    kernel_pdir[1023] = ((uint32_t)kernel_pdir) | 3;

    /* Set first page table */
    kernel_pdir[0] = (uint32_t)first_page_table | 3;

    /* Identity map first 4 MiB of memory */
    for (uint32_t phys = 0; phys < 0x400000; phys += PAGE_SIZE) {
        map_page(phys, phys);
    }

    /* Maps the kernel heap (first 1 MiB of it) */
    for (uint32_t phys = 0; phys < 0x100000; phys += PAGE_SIZE) {
        map_page(0xF0000000 + phys, 0);
    }

    enable_paging();
}

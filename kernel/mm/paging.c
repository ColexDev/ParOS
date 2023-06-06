#include <stdint.h>

#include "paging.h"
#include "pmm.h"
#include "../stdlib/util.h"
#include "../drivers/tty.h"

static uint32_t kernel_pdir[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));
static uint32_t first_page_table[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));

static uint8_t paging_enabled = 0;

static uint8_t use_global = 0;

static uint32_t* global_page_directory = kernel_pdir;

void
use_global_page_directory(uint8_t enabled)
{
    use_global = enabled;
}

void
set_page_directory(uint32_t* page_directory)
{
    global_page_directory = page_directory;
}

void
map_page(uint32_t virt, uint32_t phys)
{
    uint32_t* page_directory;
    uint32_t page_table;

    if (paging_enabled && use_global == 0)
        page_directory = (uint32_t*)VMM_PAGE_DIR;
    else
        page_directory = global_page_directory;

    if (!(page_directory[PAGE_DIRECTORY_INDEX(virt)] & 1)) {
        create_page_table(virt);
    }

    if (phys == 0) {
        phys = pmm_alloc_frame();
    }

    if (paging_enabled && use_global == 0)
        page_table = VMM_PAGE_TABLES + ((virt >> 12) * 4);
    else 
        page_table = (page_directory[PAGE_DIRECTORY_INDEX(virt)] & PAGE_TABLE_ADDRESS_MASK);

    if (paging_enabled && use_global == 0)
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

    if (paging_enabled && use_global == 0)
        page_directory = (uint32_t*)VMM_PAGE_DIR;
    else
        page_directory = global_page_directory;

    page_directory[PAGE_DIRECTORY_INDEX(virt)] = (new_table * PAGE_SIZE) | 3;

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

    if (paging_enabled && use_global == 0)
        page_directory = (uint32_t*)VMM_PAGE_DIR;
    else
        page_directory = global_page_directory;

    if (paging_enabled && use_global == 0)
        page_table = VMM_PAGE_TABLES + ((virt >> 12) * 4);
    else 
        page_table = (page_directory[PAGE_DIRECTORY_INDEX(virt)] & PAGE_TABLE_ADDRESS_MASK);

    /* Remove control bits, this gets the tables address */
    page_table &= PAGE_TABLE_ADDRESS_MASK;

    /* I think I should always do this one, the above just gets 
     * the first entry in the page table */
    page = ((uint32_t*)page_table)[PAGE_TABLE_INDEX(virt)];

    /* Remove control bits, this gets the pages address */
    page &= PAGE_TABLE_ADDRESS_MASK;

    return page;
}

void 
enable_paging(uint32_t* page_directory)
{
    uint32_t cr0 = 0;

    /* Load page directory base address into CR3 */
    asm volatile("mov %0, %%cr3":: "r"(page_directory));

    /* Enable paging by setting the paging bit in CR0 */
    asm volatile("mov %%cr0, %0; \
                  orl %1, %0; \
                  mov %0, %%cr0" 
                  : "=r"(cr0) 
                  : "i"((1 << CR0_PG_BIT)));

    paging_enabled = 1;
}

uint32_t*
create_page_directory()
{
    uint32_t* page_directory = (uint32_t*)pmm_alloc_frame();
    uint32_t* first_page_table = (uint32_t*)pmm_alloc_frame();

    for(uint16_t i = 0; i < 1024; i++) {
        page_directory[i] = 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
    }
    /* Setup recursive paging */
    page_directory[1023] = ((uint32_t)page_directory) | 3;

    /* Set first page table */
    page_directory[0] = (uint32_t)first_page_table | 3;

    return page_directory;
}

void
map_kernel_into_page_directory(uint32_t* page_directory)
{
    /* Identity map first 4 MiB of memory */
    for (uint32_t phys = 0; phys < 0x400000; phys += PAGE_SIZE) {
        map_page(phys, phys);
    }

    /* Maps the kernel heap (first 1 MiB of it) */
    for (uint32_t phys = 0; phys < 0x100000; phys += PAGE_SIZE) {
        map_page(KERNEL_HEAP_START + phys, 0);
    }
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

    map_kernel_into_page_directory(kernel_pdir);

    enable_paging(kernel_pdir);
}

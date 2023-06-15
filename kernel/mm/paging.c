#include <stdint.h>

#include "paging.h"
#include "pmm.h"
#include "../stdlib/util.h"
#include "../drivers/tty.h"
#include "../timer/timer.h"

static uint32_t kernel_pdir[TABLES_PER_DIR] __attribute__((aligned(PAGE_FRAME_SIZE)));
static uint32_t kernel_pdir_static[TABLES_PER_DIR] __attribute__((aligned(PAGE_FRAME_SIZE)));
static uint32_t first_page_table[PAGES_PER_TABLE] __attribute__((aligned(PAGE_FRAME_SIZE)));

uint32_t* page_directory_ptr = kernel_pdir_static;

void
set_page_directory_ptr(uint32_t* page_directory)
{
    page_directory_ptr = page_directory;
}

void
map_page(uint32_t virt, uint32_t phys, uint8_t get_phys)
{
    uint32_t page_table;

    if (!(page_directory_ptr[PAGE_DIRECTORY_INDEX(virt)] & 1)) {
        kprintf("Creating page table\n");
        create_page_table(virt, page_directory_ptr);
    }

    if (get_phys) {
        phys = pmm_alloc_frame();
        kprintf("Getting phys address\n");
    } else {
        // pmm_set_frame(phys / 0x1000);
        // kprintf("Setting phys frame\n");
    }

    kprintf("mapping virt 0x%x to phys 0x%x\n", virt, phys);
    page_table = (page_directory_ptr[PAGE_DIRECTORY_INDEX(virt)] & PAGE_STRUCTURE_ADDRESS_MASK);

    ((uint32_t*)page_table)[PAGE_TABLE_INDEX(virt)] = phys | 3;

    /* Flush the TLB entry for the virtual address */
    asm volatile("invlpg (%0)" ::"r" (virt) : "memory");
}

void
create_page_table(uint32_t virt, uint32_t* page_directory)
{
    uint32_t new_table = pmm_alloc_frame();

    page_directory[PAGE_DIRECTORY_INDEX(virt)] = (new_table * PAGE_SIZE) | 3;

    memset((uint32_t*)new_table, 0, PAGES_PER_TABLE);
}

/* This needs to return 0 if not present, or some special value */
uint32_t
get_page(uint32_t virt)
{
    uint32_t page;
    uint32_t page_table;

    page_table = (page_directory_ptr[PAGE_DIRECTORY_INDEX(virt)] & PAGE_STRUCTURE_ADDRESS_MASK);

    /* Remove control bits, this gets the tables address */
    page_table &= PAGE_STRUCTURE_ADDRESS_MASK;

    /* I think I should always do this one, the above just gets 
     * the first entry in the page table */
    page = ((uint32_t*)page_table)[PAGE_TABLE_INDEX(virt)];

    /* Remove control bits, this gets the pages address */
    page &= PAGE_STRUCTURE_ADDRESS_MASK;

    return page;
}

void 
enable_paging(uint32_t* page_directory)
{
    uint32_t cr0 = 0;

    /* Flush TLB */
    asm volatile("mov %cr3, %eax\n\t"
                 "mov %eax, %cr3");

    /* Load page directory base address into CR3 */
    asm volatile("mov %0, %%cr3":: "r"(page_directory));

    /* Read and modify CR0 */
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= (1 << CR0_PG_BIT);

    /* Write modified CR0 back */
    asm volatile("mov %0, %%cr0":: "r" (cr0));
}

uint32_t*
create_page_directory()
{
    uint32_t* page_directory = (uint32_t*)pmm_alloc_frame();
    kprintf("PAGE DIR ADDR PHYS: 0x%x\n", page_directory);

    // memset(page_directory, 2, TABLES_PER_DIR); /* attribute set to: supervisor level, read/write, not present(010 in binary) */
    //
    // /* Setup recursive paging */
    // page_directory[1023] = ((uint32_t)page_directory) | 3;
    //
    // /* Set first page table */
    // page_directory[0] = (uint32_t)first_page_table | 3;

    // memcpy(page_directory, kernel_pdir_static, PAGE_SIZE);

    return page_directory;
}

void
map_kernel_into_page_directory(uint32_t* page_directory)
{
    /* Identity map first 4 MiB of memory */
    for (uint32_t phys = 0; phys < 0x400000; phys += PAGE_SIZE) {
        map_page(phys, phys, 0);
    }

    /* Maps the kernel heap (first 1 MiB of it) */
    for (uint32_t phys = 0; phys < 0x100000; phys += PAGE_SIZE) {
        map_page(KERNEL_HEAP_START + phys, 0, 1);
    }
}

void
init_paging()
{
    /* Sets all page tables to not present */
    memset(kernel_pdir_static, 2, TABLES_PER_DIR); /* attribute set to: supervisor level, read/write, not present(010 in binary) */

    /* Setup recursive paging */
    kernel_pdir_static[1023] = ((uint32_t)kernel_pdir_static) | 3;

    /* Set first page table */
    kernel_pdir_static[0] = (uint32_t)first_page_table | 3;

    map_kernel_into_page_directory(kernel_pdir_static);

    set_page_directory_ptr(kernel_pdir);

    memcpy(kernel_pdir, kernel_pdir_static, PAGE_SIZE);

    enable_paging(kernel_pdir);
}

#include <stdint.h>

#include "paging.h"
#include "pmm.h"
#include "../stdlib/util.h"
#include "../stdlib/util.h"
#include "../drivers/tty.h"

uint32_t kernel_pdir[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));
uint32_t first_page_table[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));

extern uint32_t used_memory;

void
map_page(uint32_t virt, uint32_t phys)
{
    uint32_t* page_table;

    if (!(kernel_pdir[PAGE_DIRECTORY_INDEX(virt)] & 1)) {
        create_page_table(virt);
    }

    if (phys == 0) {
        phys = pmm_find_free_frame() * 0x1000;
    }

    kprintf("ACCESSING PAGE TABLE WITH VIRT: 0x%x and PHYS 0x%x\n", virt, phys);
    /* FIXME: PAGE FAULT HAPPENING HERE WHEN I DON'T ALLOC THE FIRST PAGE (0) */
    page_table = (uint32_t*)(kernel_pdir[PAGE_DIRECTORY_INDEX(virt)] & PAGE_TABLE_ADDRESS_MASK);

    /* here I am just mapping, not necessarily using the memory, so I think I
     * should not set it used here */

    kprintf("ALLOCING with VIRT: 0x%x and PHYS: 0x%x\n", virt, phys);
    pmm_set_frame(phys / 0x1000);
    used_memory += 4096;

    kprintf("Putting entry in table with VIRT: 0x%x and PHYS: 0x%x\n", virt, phys);
    page_table[PAGE_TABLE_INDEX(virt)] = phys | 3;
}

/* HAVE TO MAP PAGE ALIGNED ADDRESSES */
// uint32_t
// map_page(uint32_t virt, uint32_t phys, uint8_t is_kernel, uint8_t is_writeable)
// {
//     uint32_t page;
//     uint32_t table = kernel_pdir[PAGE_DIRECTORY_INDEX(virt)];
//
//     // if (get_page(virt, 1) != 0)
//     //     return 0;
//
//     /* Table does not exist */
//     if (!(table & PAGE_PRESENT)) {
//         /* Create table */
//     } else {
//         /* Remove control bits, this gets the tables address */
//         table &= PAGE_TABLE_ADDRESS_MASK;
//     }
//
//     /* Allocate the page */
//     alloc_page(&page, is_kernel, is_writeable);
//
//     /* Put the page in the table */
//     ((uint32_t*)table)[PAGE_TABLE_INDEX(virt)] = page;
//
//     /* Flush the TLB entry for the virtual address */
//     asm volatile("invlpg (%0)" ::"r" (virt) : "memory");
// }

void
create_page_table(uint32_t virt)
{
    uint32_t new_table = pmm_find_free_frame();
    pmm_set_frame(new_table);
    kprintf("NEW TABLE: 0x%x\n", new_table * 0x1000);

    kernel_pdir[PAGE_DIRECTORY_INDEX(virt)] = (new_table & PAGE_TABLE_ADDRESS_MASK) | 3;

    for (int i = 0; i < 1024; i++) {
        ((uint32_t*)new_table)[i] = 0;
    }
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
    uint32_t frame;
    if (GET_PAGE_FRAME(*page) != 0) /* Already alloced */
        return;

    frame = pmm_find_free_frame();

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

    /* This is currently mapping KERNEL_VIRT_BASE to virtual address 0 since it is
     * the first entry in the page table 
     * REMEMBER: The goal is to map VIRTUAL ADDRESS 0xC0000000 to PHYSICAL address 0.
     * The kernel itself starts at PHYS 0x100000 (1 MiB) as specified in the linker
     * script, but we also want stuff like the VGA buffer (PHYHS 0xB8000) to be part
     * of the kernel mapping.
     * I should create a function to map virtual addresses to physical addresses */

    /* Identity map first 4 MiB of memory */
    // kernel_pdir[1023] = (uint32_t)kernel_pdir | 3;
    kernel_pdir[0] = (uint32_t)first_page_table | 3;
    for (int i = 0; i < 1024; i++) {
        map_page(i * 0x1000, i * 0x1000);
    }
    kprintf("BEFORE: First page table 0x%x\n Second page table 0x%x\n", &kernel_pdir[0], &kernel_pdir[768]);

    enable_paging();

    /* FIXME:
     * Make a specific area of virtual memory set aside for page structures so
     * that it won't get overwritten by anything else. Basically just make a 
     * whole map of where all kernel data structures get reserved space to. And
     * don't give out those addresses
     * Only needs to be 4 MiB, so it can be from 0xC0000000 - 0xC0400000*/

    /* TESTING */
    map_page(0xC0000000, 0);
    map_page(0xE0000000, 0);
    map_page(0xF0000000, 0);
    uint32_t* arr = (uint32_t*)0xC0000000;
    arr[0] = 5;
    arr[1] = 4;
    arr[2] = 3;
    arr[3] = 2;
    arr[4] = 1;
    for (int i = 0; i < 5; i++) {
        kprintf("arr[%d] = %d\n", i, arr[i]);
    }

    kprintf("AFTER: First page table 0x%x\n Second page table 0x%x\n", &kernel_pdir[0], &kernel_pdir[1]);
    kprintf("First table addy 0x%x\n", &first_page_table);


    // char buf[16];
    // itoa(get_page(0x1000 * 1023, 1), buf, 16);
    // puts("Page: 0x");
    // puts(buf);
    // puts("\n");
}

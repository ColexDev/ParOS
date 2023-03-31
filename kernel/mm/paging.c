#include <stdint.h>

#include "paging.h"
#include "pmm.h"
#include "../stdlib/util.h"
#include "../stdlib/util.h"
#include "../drivers/tty.h"

uint32_t kernel_pdir[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));
uint32_t first_page_table[1024] __attribute__((aligned(PAGE_FRAME_SIZE)));

extern uint32_t used_memory;
uint8_t paging_enabled = 0;

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
        phys = pmm_find_free_frame() * 0x1000;
    }

    // kprintf("ACCESSING PAGE TABLE WITH VIRT: 0x%x and PHYS 0x%x\n", virt, phys);
    if (paging_enabled)
        page_table = 0xFFC00000 + ((virt >> 12) * 4);
    else 
        page_table = (page_directory[PAGE_DIRECTORY_INDEX(virt)] & PAGE_TABLE_ADDRESS_MASK);

    // kprintf("ALLOCING with VIRT: 0x%x and PHYS: 0x%x | In table: 0x%x\n", virt, phys, page_table);
    pmm_set_frame(phys / 0x1000);
    used_memory += 4096;

    // kprintf("Putting entry in table 0x%x with VIRT: 0x%x and PHYS: 0x%x\n", page_table, virt, phys);
    if (paging_enabled)
        *((uint32_t*)page_table) = phys | 3;
    else
        ((uint32_t*)page_table)[PAGE_TABLE_INDEX(virt)] = phys | 3;
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
    uint32_t* page_directory;
    uint32_t new_table = pmm_find_free_frame();

    pmm_set_frame(new_table);

    if (paging_enabled)
        page_directory = (uint32_t*)0xFFFFF000;
    else
        page_directory = kernel_pdir;

    // kprintf("NEW TABLE: 0x%x\n", new_table * 0x1000);

    page_directory[PAGE_DIRECTORY_INDEX(virt)] = (new_table * 0x1000) | 3;

    for (int i = 0; i < 1024; i++) {
        ((uint32_t*)new_table)[i] = 0;
    }
}

/* This needs to return 0 if not present, or some special value */
uint32_t
get_page(uint32_t virt, uint8_t create)
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
    // kprintf("GETTING PAGE FROM TABLE: 0x%x\n", page_table);

    /* Remove control bits, this gets the tables address */
    page_table &= PAGE_TABLE_ADDRESS_MASK;

    if (paging_enabled)
        /* FIXME: CAUSING PAGE FAULT */
        page = *((uint32_t*)page_table);
    else
        page = ((uint32_t*)page_table)[PAGE_TABLE_INDEX(virt)];
    // kprintf("PAGE: 0x%x\n", page);
    /* Remove control bits, this gets the pages address */
    page &= PAGE_TABLE_ADDRESS_MASK;
    // kprintf("GOT PAGE: 0x%x FROM INDEX: %d\n", page, PAGE_TABLE_INDEX(virt));

    /* NOTE: Change this to check if the page is present or not once I fix the table code */
    /* If page is garbage (does not exist), return 0 */
    // if ((page >> 12) > 1023)
    //     return 0;
    // else
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
    paging_enabled = 1;
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

    /* Identity map first 4 MiB of memory */
    kernel_pdir[0] = (uint32_t)first_page_table | 3;
    kernel_pdir[1023] = ((uint32_t)kernel_pdir) | 3;
    for (int i = 0; i < 1024; i++) {
        map_page(i * 0x1000, i * 0x1000);
    }

    enable_paging();

    map_page(0x1920000, 0x1920000);

    /* NOTE: DOING THE BELOW EXAMPLE IMPLEMENTATION HELPED,
     * https://forum.osdev.org/viewtopic.php?f=15&t=19387 THIS HAD WHAT I NEEDED 
     * I was just calculating the page table index wrong, its 0xFFC00000 + ((virt >> 12) * 4)
     * for some reason???*/
    /* My implementation of the above is below */
    uint32_t* page_directory = (uint32_t*)0xFFFFF000;
    uint32_t* page_table_start = (uint32_t*)0xFFC00000;
    uint32_t addr = 0x1920000;

    /* Create page table */
    // uint32_t new_table = pmm_find_free_frame();
    // kprintf("Creating Page Table: 0x%x\n", new_table);
    // page_directory[PAGE_DIRECTORY_INDEX(addr)] = (new_table * 0x1000) | 3;
    //
    // /* Map! */
    // kprintf("Mapping1\n");
    // uint32_t page_table = (uint32_t)page_table_start + ((addr >> 12) * 4);
    // kprintf("Mapping2\n");
    // *((uint32_t*)page_table) = addr | 3;
    // kprintf("Done\n");


    /* Testing it! */
    kprintf("0x1920000 loc: 0x%x\tPHYS: 0x%x\n", &page_directory[PAGE_DIRECTORY_INDEX(0x1920000)], get_page(0x1920000, 0));

    /* FIXME: Because of the behavior described below, that means after paging is enabled my 
     * paging system is messed up. I am not sure what I am doing wrong with recursive paging,
     * FIX: *********I AM NOT CLEARING THE CONTROL BITS WHEN DOING RECURSIVE PAGE TABLES *******
     * TRY TO IMPLEMENT THE SIMPLEST MAPPING AND GETTER I CAN DOWN BELOW */

    /* This DOES NOT page fault (identity mapped address) */
    uint32_t* arr2 = (uint32_t*)(1000 * 0x1000);
    arr2[0] = 15;
    kprintf("%d\n", arr2[0]);

    /* This page faults */
    uint32_t* arr = (uint32_t*)0x1920000;
    arr[0] = 16;
    kprintf("%d\n", arr[0]);


    /* FIXME: NOTE: I need recursive paging, when I have to create a new page table things go to choas since
     * it is not mapped yet, and mapping it trys to create another table... etc 
     * for some reason, if I try to identity map everything it WORKS!!
     * WAIT. This is because I am ACCIDENTLY recursively mapping the PAGE TABLE (screenshot saved in home dir) */


    /* FIXME:
     * Make a specific area of virtual memory set aside for page structures so
     * that it won't get overwritten by anything else. Basically just make a 
     * whole map of where all kernel data structures get reserved space to. And
     * don't give out those addresses
     * Only needs to be 4 MiB, so it can be from 0xC0000000 - 0xC0400000 */
    // uint32_t* page_tables = (uint32_t*) (0xFFC00000 + ((0xC0001000 >> 10) & 0xFFC));
    /* NOTE: TESTING */
    map_page(0xC0000000, 0);
    map_page(0xC0001000, 0);
    map_page(0xC0002000, 0);
    map_page(0xD0000000, 0);
    map_page(0xE0000000, 0);
    map_page(0xF0000000, 0);
    map_page(0xFF000000, 0);

    /* WHY THE FUCK ARE THESE POINTING TO THE SAME PHYSICAL ADDRESES */
    uint32_t* arr0 = (uint32_t*)0xC0000000;
    arr0[0] = 9;
    arr0[1] = 8;
    arr0[2] = 7;
    arr0[3] = 6;
    arr0[4] = 5;

    uint32_t* arr1 = (uint32_t*)0xC0001000;
    arr1[0] = 5;
    arr1[1] = 4;
    arr1[2] = 3;
    arr1[3] = 2;
    arr1[4] = 1;
    for (int i = 0; i < 5; i++) {
        kprintf("0xC0000000: arr[%d] = %d | 0xC0001000: arr1[%d] = %d\n", i, arr0[i], i, arr1[i]);
    }

    // kprintf("First page table 0x%x\n Second page table 0x%x\n", first_page_table, &kernel_pdir[1]);
    kprintf("0x3afbff   loc: 0x%x\tPHYS: 0x%x\n", &kernel_pdir[PAGE_DIRECTORY_INDEX(0x3afbff)],   get_page(0x3afbff, 0));
    kprintf("0x3fffff   loc: 0x%x\tPHYS: 0x%x\n", &kernel_pdir[PAGE_DIRECTORY_INDEX(0x3fffff)],   get_page(0x3fffff, 0));
    kprintf("0xC0000000 loc: 0x%x\tPHYS: 0x%x\n", &kernel_pdir[PAGE_DIRECTORY_INDEX(0xC0000000)], get_page(0xC0000000, 0));
    kprintf("0xC0001000 loc: 0x%x\tPHYS: 0x%x\n", &kernel_pdir[PAGE_DIRECTORY_INDEX(0xC0001000)], get_page(0xC0001000, 0));
    kprintf("0xD0000000 loc: 0x%x\tPHYS: 0x%x\n", &kernel_pdir[PAGE_DIRECTORY_INDEX(0xD0000000)], get_page(0xD0000000, 0));
    kprintf("0xC0000000 page dir index: 0x%x\tpage table index: 0x%x\n", PAGE_DIRECTORY_INDEX(0xC0000000), PAGE_TABLE_INDEX(0xC0000000));
    kprintf("0xD0000000 page dir index: 0x%x\tpage table index: 0x%x\n", PAGE_DIRECTORY_INDEX(0xD0000000), PAGE_TABLE_INDEX(0xC0000000));
    kprintf("0xE0000000 loc: 0x%x\tPHYS: 0x%x\n", &kernel_pdir[PAGE_DIRECTORY_INDEX(0xE0000000)], get_page(0xE0000000, 0));
    kprintf("0xF0000000 loc: 0x%x\tPHYS: 0x%x\n", &kernel_pdir[PAGE_DIRECTORY_INDEX(0xF0000000)], get_page(0xF0000000, 0));
    kprintf("0xFF000000 loc: 0x%x\tPHYS: 0x%x",   &kernel_pdir[PAGE_DIRECTORY_INDEX(0xFF000000)], get_page(0xFF000000, 0));
    // kprintf("pdir[1023] loc: 0x%x\n", &kernel_pdir[1023]);
}

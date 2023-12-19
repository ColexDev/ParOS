#include <stdint.h>

#include "paging.h"
#include "pmm.h"
#include "../stdlib/util.h"
#include "../drivers/tty.h"
#include "../timer/timer.h"

static uint32_t kernel_pdir[TABLES_PER_DIR] __attribute__((aligned(PAGE_FRAME_SIZE)));
static uint32_t kernel_pdir_static[TABLES_PER_DIR] __attribute__((aligned(PAGE_FRAME_SIZE)));
static uint32_t first_page_table[PAGES_PER_TABLE] __attribute__((aligned(PAGE_FRAME_SIZE)));

// uint32_t* page_directory_ptr = kernel_pdir_static;

extern uint32_t kernel_end;
extern uint32_t kernel_start;
extern uint32_t text_start;
extern uint32_t text_end;
extern uint32_t rodata_start;
extern uint32_t rodata_end;
extern uint32_t data_start;
extern uint32_t data_end;
extern uint32_t bss_start;
extern uint32_t bss_end;

// void
// set_page_directory_ptr(uint32_t* page_directory)
// {
//     page_directory_ptr = page_directory;
// }

/* TODO: If page_directory is NULL, use recursive mapping addr 0xFFFFF000 */
/* FIXME: Do NOT allocate physical memory or set any physical memory in this function, create an allocate page function for that (in do checks for taken virtual and physical memory as well)!!! Also this does not check to see if the physical address is already taken */
void
map_page(uint32_t* page_directory, uint32_t virt, uint32_t phys, uint8_t get_phys)
{
    if (!(page_directory[PAGE_DIRECTORY_INDEX(virt)] & 1)) {
        create_page_table(page_directory, virt);
    }

    if (get_phys) {
        phys = pmm_alloc_frame();
    } else {
        /* FIXME: nooo this is bad news bears */
        pmm_set_frame(phys / 0x1000);
    }

    uint32_t* page_table = (uint32_t*)(page_directory[PAGE_DIRECTORY_INDEX(virt)] & PAGE_STRUCTURE_ADDRESS_MASK);

    page_table[PAGE_TABLE_INDEX(virt)] = phys | 3;

    /* Flush the TLB entry for the virtual address */
    asm volatile("invlpg (%0)" ::"r" (virt) : "memory");
}

void
create_page_table(uint32_t* page_directory, uint32_t virt)
{
    uint32_t new_table = pmm_alloc_frame();
    // kprintf("New page table at phys: 0x%x when allocing virt addr 0x%x\n", new_table, virt);

    page_directory[PAGE_DIRECTORY_INDEX(virt)] = (new_table * PAGE_SIZE) | 3;

    memset((uint32_t*)new_table, 0, PAGES_PER_TABLE);
}

/* This needs to return 0 if not present, or some special value */
uint32_t
get_page(uint32_t* page_directory, uint32_t virt)
{
    uint32_t page_table = (page_directory[PAGE_DIRECTORY_INDEX(virt)] & PAGE_STRUCTURE_ADDRESS_MASK);

    /* Remove control bits, this gets the tables address */
    page_table &= PAGE_STRUCTURE_ADDRESS_MASK;

    /* I think I should always do this one, the above just gets 
     * the first entry in the page table */
    uint32_t page = ((uint32_t*)page_table)[PAGE_TABLE_INDEX(virt)];

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
    // kprintf("PAGE DIR ADDR PHYS: 0x%x\n", page_directory);
    
    // memset(page_directory, 2, TABLES_PER_DIR); /* attribute set to: supervisor level, read/write, not present(010 in binary) */
    //
    // /* Setup recursive paging */
    // page_directory[1023] = ((uint32_t)page_directory) | 3;

    memcpy(page_directory, kernel_pdir_static, PAGE_SIZE);

    return page_directory;
}

void
map_kernel_into_page_directory(uint32_t* page_directory)
{
    /* FIXME: If I don't wanna identity map this much I can get loc of memmap too and map it */
    /* Identity map first 1 MiB of memory */
    // for (uint32_t phys = 0; phys < 0x1000; phys += PAGE_SIZE) {
    //     map_page(page_directory, phys, phys, 0);
    // }

    /* Identity map 0 so that we don't page fault upon reading NULL */
    map_page(page_directory, 0x0, 0x0, 0);

    /* FIXME: This is a fixed address for grub memory map, update it to get the actual addr */
    for (uint32_t phys = 0x1009c; phys < 0x10144; phys += PAGE_SIZE) {
        map_page(page_directory, phys, phys, 0);
    }

    /* FIXME: This is a fixed address for reserved memory, update it to get the actual addr */
    for (uint32_t phys = 0x9fc00; phys < 0xfffff; phys += PAGE_SIZE) {
        map_page(page_directory, phys, phys, 0);
    }

    /* Maps the kernel (text, rodata, data, bss) */
    for (uint32_t phys = (uint32_t)&kernel_start; phys < ALIGN_DOWN((uint32_t)&kernel_end); phys += PAGE_SIZE) {
        map_page(page_directory, phys, phys, 0);
    }

    /* Maps VGA buffer */
    map_page(page_directory, 0xB8000, 0xB8000, 0);

    /* Maps first page of kernel heap */
    // map_page(page_directory, KERNEL_HEAP_START, 0, 1);
    

    /* Maps first 8 pages of kernel heap */
    for (uint32_t phys = KERNEL_HEAP_START; phys < KERNEL_HEAP_START + (PAGE_SIZE * 8); phys += PAGE_SIZE) {
        map_page(page_directory, phys, 0, 1);
    }
}

void
init_paging()
{
    /* Sets all page tables to not present */
    // memset(kernel_pdir_static, 2, TABLES_PER_DIR); /* attribute set to: supervisor level, read/write, not present(010 in binary) */
    memset(kernel_pdir, 2, TABLES_PER_DIR); /* attribute set to: supervisor level, read/write, not present(010 in binary) */

    /* Setup recursive paging */
    // kernel_pdir_static[1023] = ((uint32_t)kernel_pdir_static) | 3;
    kernel_pdir[1023] = (uint32_t)kernel_pdir | 3;

    /* Set first page table */
    // kernel_pdir_static[0] = (uint32_t)first_page_table | 3;
    kernel_pdir[0] = (uint32_t)first_page_table | 3;

    // map_kernel_into_page_directory(kernel_pdir_static);

    map_kernel_into_page_directory(kernel_pdir);

    // memcpy(kernel_pdir, kernel_pdir_static, PAGE_SIZE);

    // kprintf("Kernel start: 0x%x\n", &kernel_start);
    // kprintf("Kernel end: 0x%x\n", &kernel_end);
    // kprintf("Kernel end aligned down: 0x%x\n", ALIGN_DOWN((uint32_t)&kernel_end));
    // kprintf("Text start: 0x%x\n", &text_start);
    // kprintf("Text end: 0x%x\n", &text_end);
    // kprintf("Rodata start: 0x%x\n", &rodata_start);
    // kprintf("Rodata end: 0x%x\n", &rodata_end);
    // kprintf("Data start: 0x%x\n", &data_start);
    // kprintf("Data end: 0x%x\n", &data_end);
    // kprintf("BSS start: 0x%x\n", &bss_start);
    // kprintf("BSS end: 0x%x\n", &bss_end);

    enable_paging(kernel_pdir);
}

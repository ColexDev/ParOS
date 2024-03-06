#include <mem/pmm/pmm.h>
#include <io/printf.h>
#include <mem/memmap/memmap.h>
#include <bl/bl.h>
#include <klibc/string/string.h>
#include <cpu/interrupts/interrupts.h>
#include <debug/debug.h>

#include "vmm.h"

struct page_table* kernel_pml4;

void
load_pml4(const struct page_table* pml4)
{
    struct page_table* pml4_phys = (struct page_table*)((uint64_t)pml4 - bl_get_hhdm_offset());

    asm volatile("mov %0, %%cr3" : : "r" (pml4_phys));
}

void
page_fault_handler(struct interrupt_frame* frame)
{
    uint64_t faulting_address;
    uint64_t err_code = frame->error_code;
    asm volatile ("mov %%cr2, %0" : "=r" (faulting_address));
    kprintf("Error code: %d\n", err_code);
    /* Protection error */
    if (err_code & 1) {
        kprintf("Protection issue\n");
    /* Page not present */
    } else if (err_code & 2) {
        kprintf("Page not present...\n");
        kprintf("Faulting address: 0x%llx\n", faulting_address);
        print_stack_trace();
    } else if (err_code & 4) {
        kprintf("Reserved bit violation...\n");
    } else if (err_code & 8) {
        kprintf("User mode issue...\n");
    }

    kprintf("PML4[%lld]->PDP[%lld]->PD[%lld]->PT[%lld]\n",
            PML4_IDX(faulting_address), PDP_IDX(faulting_address), PD_IDX(faulting_address), PT_IDX(faulting_address));
    // kprintf("PML4[%lld]->PDP[%lld]->PD[%lld]->PT[%lld] = 0x%llx\n",
            // PML4_IDX(faulting_address), PDP_IDX(faulting_address), PD_IDX(faulting_address), PT_IDX(faulting_address), pt->entries[PT_IDX(faulting_address)]);
    // kprintf("PML4[%lld] = 0x%llx\n", PML4_IDX(faulting_address), pml4->entries[PML4_IDX(faulting_address)]);
    // kprintf("PDP[%lld] = 0x%llx\n", PDP_IDX(faulting_address), pdp->entries[PDP_IDX(faulting_address)]);
    // kprintf("PD[%lld] = 0x%llx\n", PD_IDX(faulting_address), pd->entries[PD_IDX(faulting_address)]);
    // kprintf("PT[%lld] = 0x%llx\n", PT_IDX(faulting_address), pt->entries[PT_IDX(faulting_address)]);
    kprintf("PML4[%lld]\n", PML4_IDX(faulting_address));
    kprintf("PDP[%lld]\n", PDP_IDX(faulting_address));
    kprintf("PD[%lld]\n", PD_IDX(faulting_address));
    kprintf("PT[%lld]\n", PT_IDX(faulting_address));

    for (;;) {
        asm("nop");
    }
    print_stack_trace();
}

uint8_t
dump_pte(const uint64_t virt)
{
    struct page_table* pml4 = kernel_pml4;
    struct page_table* pdp;
    struct page_table* pd;
    struct page_table* pt;

    if (!(pml4->entries[PML4_IDX(virt)] & PTE_PRESENT)) {
        kprintf("DNE\n");
        return 1;
    } else {
        pdp = (struct page_table*)(PTE_GET_ADDR(pml4->entries[PML4_IDX(virt)]) + bl_get_hhdm_offset());
    }

    if (!(pdp->entries[PDP_IDX(virt)] & PTE_PRESENT)) {
        kprintf("DNE\n");
        return 2;
    } else {
        pd = (struct page_table*)(PTE_GET_ADDR(pdp->entries[PDP_IDX(virt)]) + bl_get_hhdm_offset());
    }

    if (!(pd->entries[PD_IDX(virt)] & PTE_PRESENT)) {
        kprintf("DNE\n");
        return 3;
    } else {
        pt = (struct page_table*)(PTE_GET_ADDR(pd->entries[PD_IDX(virt)]) + bl_get_hhdm_offset());
    }

    if (!(pt->entries[PT_IDX(virt)] & PTE_PRESENT)) {
        kprintf("DNE\n");
        return 4;
    } else {
        kprintf("virt (0x%llx) -> entry (0x%llx)\n", virt, pt->entries[PT_IDX(virt)]);
    }

    return 0;
}

void
vmm_init()
{
    register_interrupt_handler(14, page_fault_handler);

    kernel_pml4 = (struct page_table*)(pmm_alloc(1) + bl_get_hhdm_offset());
    kprintf("pml4 phys: 0x%llx\n", ((uint64_t)kernel_pml4 - bl_get_hhdm_offset()));
    kprintf("pml4 virt: 0x%llx\n", kernel_pml4);
    struct memmap_entry entry;
    for(uint64_t i = 0; i < memmap_get_num_entries(); i++){
        entry = memmap_get_entry(i);

        if (entry.type == MEMMAP_KERNEL_AND_MODULES) {
            // kprintf("===MAPPING KERNEL phys 0x%llx at virt 0x%llx===\n", 
                    // bl_get_kernel_phys_addr(), bl_get_kernel_virt_addr());
            for(uint64_t i = 0; i <= ALIGN_UP(entry.length, PAGE_SIZE); i += PAGE_SIZE){
                vmm_map_page(kernel_pml4, i + bl_get_kernel_phys_addr(), i + bl_get_kernel_virt_addr());
            }
        }
    }

    // map first 4 GiB
    // kprintf("===MAPPING FIRST 4GB===\n");
    for(uint64_t i = 0x1000; i <= 0xffffffff; i += PAGE_SIZE){
        // vmm_map_page(kernel_pml4, i, i);
        vmm_map_page(kernel_pml4, i, i + bl_get_hhdm_offset());
    }

    load_pml4(kernel_pml4);
}


/* FIXME: 
 * Currently when accessing the page tables in the else statements I do NOT add
 * the HHDM offset, this will cause issues if I do not identity map the first 4GiB and 
 * only map it at the HHDM offset which I will probably do, I need to go in and
 * add the HHDM in the else statements as well, not sure why I did it when creating them
 * and not in the else */
uint64_t
vmm_map_page(struct page_table* pml4, const uint64_t phys, const uintptr_t virt)
{
    /* add ability to pass in flags, used for USER flag probably */
    
    struct page_table* pdp;
    struct page_table* pd;
    struct page_table* pt;

    /* FIXME: 2024-02-17:
     * Add HHDM to pdp, pd, pt AFTER allocation and just subtract it when adding
     * into top level, this needs to be done for accessing things AFTER switching
     * to new kernel pagedir (mine). Example of a correct but messy way is for the PDP,
     * it works, others do not, so modify it.
    */

    // kprintf("===MAPPING 0x%llx to 0x%llx\n===", phys, virt);
    if (!(pml4->entries[PML4_IDX(virt)] & PTE_PRESENT)) {
        // kprintf("NOT PRESENT\n");
        pdp = pmm_alloc(1);
        // kprintf("ALLOC PDP: 0x%llx\n", pdp);
        uint64_t pdp_virt = (uint64_t)pdp + bl_get_hhdm_offset();
        // kprintf("ALLOC PDP VIRT: 0x%llx\n", pdp_virt);
        memset((void*)pdp_virt, 0, PAGE_SIZE);
        // kprintf("MEMSET PDP to 0\n");

        pml4->entries[PML4_IDX(virt)] = (uint64_t)pdp | PTE_PRESENT | PTE_WRITABLE;

        pdp = (struct page_table*)((uint64_t)pdp + bl_get_hhdm_offset());
        // kprintf("pdp virt: 0x%llx\n", pdp);
    } else {
        pdp = (struct page_table*)(PTE_GET_ADDR(pml4->entries[PML4_IDX(virt)]) + bl_get_hhdm_offset());
    }

    // kprintf("PDP entry: 0x%llx\n", pdp->entries[PDP_IDX(virt)]);
    if (!(pdp->entries[PDP_IDX(virt)] & PTE_PRESENT)) {
        pd = pmm_alloc(1);
        memset(pd, 0, PAGE_SIZE);
        // kprintf("pd phys: 0x%llx\n", pd);

        pdp->entries[PDP_IDX(virt)] = (uint64_t)pd | PTE_PRESENT | PTE_WRITABLE;

        pd = (struct page_table*)((uint64_t)pd + bl_get_hhdm_offset());
    } else {
        pd = (struct page_table*)(PTE_GET_ADDR(pdp->entries[PDP_IDX(virt)]) + bl_get_hhdm_offset());
    }

    // kprintf("PD entry: 0x%llx\n", pd->entries[PD_IDX(virt)]);
    if (!(pd->entries[PD_IDX(virt)] & PTE_PRESENT)) {
        pt = pmm_alloc(1);
        memset(pt, 0, PAGE_SIZE);
        // kprintf("pt phys: 0x%llx\n", pt);

        pd->entries[PD_IDX(virt)] = (uint64_t)pt | PTE_PRESENT | PTE_WRITABLE;

        pt = (struct page_table*)((uint64_t)pt + bl_get_hhdm_offset());
    } else {
        pt = (struct page_table*)(PTE_GET_ADDR(pd->entries[PD_IDX(virt)]) + bl_get_hhdm_offset());
    }

    // kprintf("PT entry: 0x%llx\n", pt->entries[PT_IDX(virt)]);
    // page_table_entry curr_entry = pt->entries[PT_IDX(virt)];

    pt->entries[PT_IDX(virt)] = phys | PTE_PRESENT | PTE_WRITABLE;
    // if (!(pt->entries[PT_IDX(virt)] & PTE_PRESENT)) {
    //     pt->entries[PT_IDX(virt)] = phys | PTE_PRESENT | PTE_WRITABLE;
    // } else {
    //     // kprintf("The above already EXISTS\n");
    // }
    // if (phys <= 0xf6000) {
    //     kprintf("===For Virt(0x%llx) -> Phys(0x%llx)===\n", virt, phys);
    //     kprintf("PML4[%lld]->PDP[%lld]->PD[%lld]->PT[%lld] = 0x%llx\n",
    //             PML4_IDX(virt), PDP_IDX(virt), PD_IDX(virt), PT_IDX(virt), pt->entries[PT_IDX(virt)]);
    //     kprintf("PML4[%lld] = 0x%llx\n", PML4_IDX(virt), pml4->entries[PML4_IDX(virt)]);
    //     kprintf("PDP[%lld] = 0x%llx\n", PDP_IDX(virt), pdp->entries[PDP_IDX(virt)]);
    //     kprintf("PD[%lld] = 0x%llx\n", PD_IDX(virt), pd->entries[PD_IDX(virt)]);
    //     kprintf("PT[%lld] = 0x%llx\n", PT_IDX(virt), pt->entries[PT_IDX(virt)]);
    // }
    // kprintf("page phys: 0x%llx\n", curr_entry);
    // kprintf("PT[%lld] entry AFTER: 0x%llx\n", PT_IDX(virt), pt->entries[PT_IDX(virt)]);
    return 0;
}


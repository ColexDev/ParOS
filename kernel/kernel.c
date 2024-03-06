#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <stdarg.h>

#include <io/serial.h>
#include <io/port_io.h>
#include <io/printf.h>
#include <io/printf.h>
#include <cpu/interrupts/idt.h>
#include <cpu/interrupts/interrupts.h>
#include <cpu/gdt/gdt.h>
#include <debug/debug.h>
#include <mem/pmm/pmm.h>
#include <mem/vmm/vmm.h>
#include <mem/memmap/memmap.h>
#include <bl/bl.h>
#include <acpi/acpi.h>
#include <cpu/interrupts/apic/apic.h>

// Halt and catch fire function. (from limine)
static void
hcf(void)
{
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}


void
_start(void)
{
    uint64_t top_of_stack;
    asm volatile ("movq %%rsp, %0" : "=r" (top_of_stack));
    gdt_init();
    idt_init();
    pmm_init();
    memmap_print();
    vmm_init();

    uint64_t offset = bl_get_hhdm_offset();
    kprintf("kernel virt: 0x%llx\t Entry phys: 0x%llx\n", 
            bl_get_kernel_virt_addr(), bl_get_kernel_phys_addr());
    kprintf("Current top of stack PHYS: 0x%llx\n", top_of_stack - offset);

    acpi_parse_tables();
    apic_init();
    kprintf("APIC init done\n");

    // We're done, just hang...
    hcf();
}

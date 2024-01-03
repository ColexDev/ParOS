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

// Halt and catch fire function. (from limine)
static void
hcf(void)
{
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static volatile struct limine_kernel_address_request kernel_addr = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

void
_start(void)
{
    uint64_t top_of_stack;
    asm volatile ("movq %%rsp, %0" : "=r" (top_of_stack));
    gdt_init();
    idt_init();
    kprintf("BEFORE INTERRUPT\n");
    print_registers();

    /* Adding all this so test_var doesn't get optimized away 
     * testing to see if stack is alright with test_var2 */
    uint64_t test_var = 5;
    uint64_t test_var2 = 12;
    test_var++;

    asm ("int $0x2");
    kprintf("AFTER INTERRUPT\n");

    uint64_t offset = hhdm_request.response->offset;
    kprintf("HHDM offset: 0x%llx\n", offset);
    kprintf("kernel virt: 0x%llx\t Entry phys: 0x%llx\n", 
            kernel_addr.response->virtual_base, kernel_addr.response->physical_base);
    kprintf("test_var2 virt: 0x%llx\t without offset: 0x%llx\n",
            &test_var2, (uint64_t)(&test_var2) - offset);
    kprintf("Top of stack PHYS: 0x%llx\n", top_of_stack - offset);
    // memmap_print();

    // We're done, just hang...
    hcf();
}

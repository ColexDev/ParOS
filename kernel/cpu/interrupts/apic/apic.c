#include <stdint.h>
#include <stddef.h>
#include <acpi/acpi.h>
#include <cpu/interrupts/pic/pic.h>
#include <io/printf.h>
#include <bl/bl.h>
#include <mem/vmm/vmm.h>

static uintptr_t ioapic_addr;

/*
 * apic_base: IOREGSEL (I/O Register Select [index])
 *  - Selects the register to read/write from
 * apic_base + 0x10: IOWIN (I/O Window [data])
 *  - Data to write to the register, or filled with data from read
*/
static void
apic_write_register(const uintptr_t apic_base, const uint8_t reg, const uint32_t val)
{
    volatile uint32_t* apic = (volatile uint32_t*)apic_base;

    apic[0] = reg;
    apic[4] = val; /* apic_base + 0x10 */
}

static uint32_t
apic_read_register(const uintptr_t apic_base, const uint8_t reg)
{
    volatile uint32_t* apic = (volatile uint32_t*)apic_base;

    apic[0] = reg;
    return apic[4]; /* apic_base + 0x10 */
}

void
apic_init(void)
{
    /* Disable the PIC */
    pic_remap();
    pic_disable();

    /* Lets parse the MADT for IOAPICs */
    struct acpi_madt_record_header* curr_record = &MADT->records;

    kprintf("Starting to parse for IOAPICs\n");
    kprintf("MADT length: %d\n", MADT->header.length);
    kprintf("MADT struct size: %d\n", sizeof(struct acpi_madt));
    kprintf("First record type: %d\n", curr_record->entry_type);
    kprintf("First record length: %d\n", curr_record->record_length);

    /* This probably goes over a bit, header.length is too long */
    for (size_t i = 0; i < MADT->header.length; i += curr_record->record_length) {
        curr_record += curr_record->record_length;

        if (curr_record->entry_type != ACPI_MADT_IOAPIC) {
            continue;
        }

        struct acpi_madt_ioapic_record* curr_ioapic = (struct acpi_madt_ioapic_record*)curr_record;
        kprintf("ID: %d\n", curr_ioapic->ioapic_id);
        kprintf("Address: 0x%x\n", curr_ioapic->address);
        kprintf("GSI: %d\n", curr_ioapic->global_system_interrupt_base);
        ioapic_addr = curr_ioapic->address + bl_get_hhdm_offset();
        vmm_map_page(kernel_pml4, curr_ioapic->address, ioapic_addr);
    }
}

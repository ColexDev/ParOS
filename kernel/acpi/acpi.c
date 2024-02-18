#include <stdint.h>
#include <stddef.h>
#include <bl/bl.h>
#include <io/printf.h>
#include <mem/vmm/vmm.h>
#include <klibc/string/string.h>

#include "acpi.h"

struct acpi_rsdp* RSDP;
struct acpi_sdt* SDT;
uint8_t acpi_version;

static uint8_t
validate_acpi_checksum(uint64_t p, size_t len)
{
    uint64_t checksum = 0;
    uint8_t* byte_arr = (uint8_t*)p;

    for (size_t i = 0; i < len; i++) {
        checksum += byte_arr[i];
    }

    if ((checksum & 0xFF) == 0) {
        return 1;
    } else {
        return 0;
    }
}

/* Taken from https://wiki.osdev.org/RSDT */
static uint64_t
acpi_find_table(const char* name)
{
    uint64_t num_entries = (SDT->header.length - sizeof(SDT->header)) / (4 * acpi_version);
    kprintf("========num_entries: %d\n", num_entries);
    kprintf("========SDT ADDR: 0x%llx\n", SDT);
    kprintf("========SDT->entries ADDR: 0x%llx\n", SDT->entries);

    for (uint64_t i = 0; i < num_entries; i++) {
        kprintf("========entry: 0x%llx\n", SDT->entries[i * acpi_version]);
        struct acpi_sdt_header* header = (struct acpi_sdt_header*)((SDT->entries)[i * acpi_version] + bl_get_hhdm_offset());
        kprintf("SIG: %s\n", header->signature);
        if (!strncmp(header->signature, name, 4)) {
            return (uint64_t)header;
        }
    }
}

void
parse_acpi_tables(void)
{
    uint64_t hhdm_offset = bl_get_hhdm_offset();

    uint64_t rsdp_addr = bl_get_rsdp_addr();

    // kprintf("RET: %d\n", dump_pte(ALIGN_DOWN(rsdp_addr, 0x1000)));

    RSDP = (struct acpi_rsdp*)(rsdp_addr);

    /* Determine if ACPI version 1 or 2 for the len (RSDP vs XSDP) */
    size_t len = (ACPI_SIZE_RSDP + (RSDP->revision / 2) * (ACPI_SIZE_XSDP));

    uint8_t res = validate_acpi_checksum((rsdp_addr), len);
    kprintf("CHECKSUM: %d\n", res);
    if (!res) {
        kprintf("INVALID RSDP TABLE\n");
    }

    /* In ACPI V2 we HAVE to use XSDT */
    if (RSDP->revision == 0) {
        SDT = (struct acpi_sdt*)(RSDP->RSDT_address + bl_get_hhdm_offset());
        acpi_version = 1;
    } else if (RSDP->revision == 2) {
        SDT = (struct acpi_sdt*)(RSDP->XSDT_address + bl_get_hhdm_offset());
        acpi_version = 2;
    }

    kprintf("ACPI Version: %d\n", acpi_version);
    kprintf("RSDT ADDR: 0x%llx\n", RSDP->RSDT_address);
    kprintf("XSDT ADDR: 0x%llx\n", RSDP->XSDT_address);

    /* MADT */
    acpi_find_table("APIC");

    kprintf("Mapping NEW PAGE\n");
    vmm_map_page(kernel_pml4, 0xFFFFFFFFF, 0xFFFFFFFFF);
}

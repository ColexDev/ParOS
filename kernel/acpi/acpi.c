#include <stdint.h>
#include <stddef.h>
#include <bl/bl.h>
#include <io/printf.h>
#include <mem/vmm/vmm.h>
#include <klibc/string/string.h>

#include "acpi.h"

/* This points to the RSDT/XSDT */
struct acpi_rsdp* RSDP;

/* XRSDT bc it can either be RSDT or XSDT */
/* This points to other descriptor tables (such as MADT) */
struct acpi_sdt* XRSDT;

/* Descriptor table for APIC */
struct acpi_madt* MADT;

uint8_t acpi_version;

static uint8_t
acpi_validate_checksum(const uint64_t p, const size_t len)
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

/* Based on https://wiki.osdev.org/RSDT */
static uint64_t
acpi_find_table(const char* name)
{
    uint64_t num_entries = (XRSDT->header.length - sizeof(XRSDT->header)) / (4 * acpi_version);
    // kprintf("========num_entries: %d\n", num_entries);
    // kprintf("========SDT ADDR: 0x%llx\n", XRSDT);
    // kprintf("========SDT->entries ADDR: 0x%llx\n", XRSDT->entries);

    for (size_t i = 0; i < num_entries; i++) {
        // kprintf("========entry: 0x%llx\n", XRSDT->entries[i * acpi_version]);
        struct acpi_sdt_header* header = (struct acpi_sdt_header*)((XRSDT->entries)[i * acpi_version] + bl_get_hhdm_offset());
        // kprintf("SIG: %s\n", header->signature);
        if (!strncmp(header->signature, name, 4)) {
            return (uintptr_t)header;
        }
    }
}

void
acpi_parse_tables(void)
{
    const uint64_t hhdm_offset = bl_get_hhdm_offset();

    uintptr_t rsdp_addr = bl_get_rsdp_addr();

    // kprintf("RET: %d\n", dump_pte(ALIGN_DOWN(rsdp_addr, 0x1000)));

    RSDP = (struct acpi_rsdp*)(rsdp_addr);

    /* Determine if ACPI version 1 or 2 for the len (RSDP vs XSDP) */
    size_t len = (ACPI_SIZE_RSDP + (RSDP->revision / 2) * (ACPI_SIZE_XSDP));

    uint8_t res = acpi_validate_checksum((rsdp_addr), len);
    if (!res) {
        kprintf("INVALID RSDP TABLE\n");
    }

    /* In ACPI V2 we HAVE to use XSDT, but in ACPI V1 we will use RSDT */
    if (RSDP->revision == 0) {
        XRSDT = (struct acpi_sdt*)(RSDP->RSDT_address + bl_get_hhdm_offset());
        /* If RSDT_address is NULL, panic */
        acpi_version = 1;
    } else if (RSDP->revision == 2) {
        XRSDT = (struct acpi_sdt*)(RSDP->XSDT_address + bl_get_hhdm_offset());
        /* If XSDT_address is NULL, panic */
        acpi_version = 2;
    }

    res = acpi_validate_checksum((uintptr_t)XRSDT, XRSDT->header.length);

    kprintf("ACPI Version: %d\n", acpi_version);
    kprintf("RSDT ADDR: 0x%llx\n", RSDP->RSDT_address);
    kprintf("XSDT ADDR: 0x%llx\n", RSDP->XSDT_address);

    /* Get APIC information table */
    MADT = (struct acpi_madt*)acpi_find_table("APIC");
    res = acpi_validate_checksum((uintptr_t)MADT, MADT->header.length);
    if (!res) {
        kprintf("INVALID MADT TABLE\n");
    }
    kprintf("LAPIC ADDR: 0x%x\n", MADT->lapic_addr);
    kprintf("%s\n", MADT->flags ? "Legacy PICs, we need to mask all the interrupts" : "No Legacy PICs");
}

#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>

#define ACPI_SIZE_RSDP 20

/* Extra stuff addes in XSDP, not whole thing */
#define ACPI_SIZE_XSDP 16

struct acpi_rsdp
{
    /* ACPI_SIZE_RSDP */
    char     signature[8];
    uint8_t  checksum;
    char     OEMID[6];
    uint8_t  revision;
    uint32_t RSDT_address;

    /* ACPI_SIZE_XSDP */
    uint32_t length;
    uint64_t XSDT_address;
    uint8_t  extended_checksum;
    uint8_t  reserved[3];
} __attribute__ ((packed));

struct acpi_sdt_header
{
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     OEMID[6];
    char     OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t creator_ID;
    uint32_t creator_revision;
} __attribute__ ((packed));

struct acpi_sdt
{
    struct acpi_sdt_header header;   
    uint32_t entries[];
} __attribute__ ((packed));

/* https://wiki.osdev.org/MADT */
#define ACPI_MADT_LAPIC  (0)
#define ACPI_MADT_IOAPIC (1)
#define ACPI_MADT_IOAPIC_INTERRUPT_SOURCE_OVERRIDE (2)

struct acpi_madt_record_header
{
    uint8_t entry_type;
    uint8_t record_length;
} __attribute__ ((packed));

struct acpi_madt
{
    struct acpi_sdt_header header;
    uint32_t lapic_addr;
    uint32_t flags;
    struct acpi_madt_record_header records;
} __attribute__ ((packed));

extern struct acpi_madt* MADT;

#define ACPI_MADT_LAPIC_PROCESSOR_ENABLED (1 << 0);
/* If ACPI_MADT_LAPIC_PROCESSOR_ENABLED is not set but this is, 
 * then the CPU can be enabled, but if this is not set, then
 * we should NOT try to enable the CPU */
#define ACPI_MADT_LAPIC_ONLINE_CAPABLE    (1 << 1);

struct acpi_madt_lapic_record
{
    struct acpi_madt_record_header header;
    uint8_t acpi_processor_id;
    uint8_t lapic_id;
    uint32_t flags;
} __attribute__ ((packed));

struct acpi_madt_ioapic_record
{
    struct acpi_madt_record_header header;
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t address;
    uint32_t global_system_interrupt_base;
} __attribute__ ((packed));

struct acpi_madt_ioapic_interrupt_source_override
{
    struct acpi_madt_record_header header;
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t global_system_interrupt;
    uint16_t flags;
} __attribute__ ((packed));

void acpi_parse_tables(void);

#endif /* ACPI_H */

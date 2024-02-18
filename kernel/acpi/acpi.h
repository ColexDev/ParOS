#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>

/* Copying some structs from osdev wiki */

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

void parse_acpi_tables(void);
#endif /* ACPI_H */

#ifndef MEMMAP_H
#define MEMMAP_H

#include <stdint.h>

#define MEMMAP_USABLE                 0
#define MEMMAP_RESERVED               1
#define MEMMAP_ACPI_RECLAIMABLE       2
#define MEMMAP_ACPI_NVS               3
#define MEMMAP_BAD_MEMORY             4
#define MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define MEMMAP_KERNEL_AND_MODULES     6
#define MEMMAP_FRAMEBUFFER            7

struct memmap_entry
{
    uint64_t base;
    uint64_t length;
    uint64_t type;
};

uint64_t memmap_get_highest_usable_address(void);
void memmap_print(void);
struct memmap_entry memmap_get_entry(uint64_t entry);
uint64_t memmap_get_num_entries(void);

#endif /* MEMMAP_H */

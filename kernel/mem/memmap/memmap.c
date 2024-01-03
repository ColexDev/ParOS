#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include <io/printf.h>

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static char* memmap_types[] = {
    "USABLE",
    "RESERVED",
    "ACPI_RECLAIMABLE",
    "ACPI_NVS",
    "BAD_MEMORY",
    "BOOTLOADER_RECLAIMABLE",
    "KERNEL_AND_MODULES",
    "FRAMEBUFFER"
};

void
memmap_print(void)
{
    struct limine_memmap_response* memmap = memmap_request.response;

    for(uint64_t i = 0; i < memmap->entry_count; i++){
        kprintf("[0x%lx->0x%lx] %luB %s\n", memmap->entries[i]->base, memmap->entries[i]->base + memmap->entries[i]->length, memmap->entries[i]->length, memmap_types[memmap->entries[i]->type]);
    }
}
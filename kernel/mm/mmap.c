#include <stdint.h>

#include "../stdlib/util.h"
#include "../multiboot.h"
#include "../drivers/tty.h"

#define KERNEL_START 0x100000 /* 1 MiB */

uint32_t total_bytes_ram;

void
parse_multiboot_mmap(multiboot_info_t* mbi)
{
    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
    kprintf("memory map addr start: 0x%x\n", mbi->mmap_addr);
    uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;
    kprintf("memory map addr end: 0x%x\n", mmap_end);

    while ((uint32_t)mmap < mmap_end) {
        uint64_t base_addr = ((uint64_t)mmap->addr_high << 32) | mmap->addr_low;
        uint64_t length = ((uint64_t)mmap->len_high << 32) | mmap->len_low;

        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            kprintf("Usable memory:   0x%llx - 0x%llx (%llu bytes)\n", base_addr, base_addr + length - 1, length);
        } else if (mmap->type == MULTIBOOT_MEMORY_RESERVED) {
            kprintf("Reserved memory: 0x%llx - 0x%llx (%llu bytes)\n", base_addr, base_addr + length - 1, length);
        } else {
            kprintf("Other memory type: 0x%llx - 0x%llx (%llu bytes)\n", base_addr, base_addr + length - 1, length);
        }

        mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + sizeof(uint32_t));
    }
}

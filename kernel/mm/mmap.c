#include <stdint.h>

#include "../stdlib/util.h"
#include "../multiboot.h"
#include "../drivers//tty.h"

#define KERNEL_START 0x100000 /* 1 MiB */

extern uint32_t curr_free_mem;

uint32_t total_bytes_ram;

void
print_mmap(multiboot_info_t* mbi)
{
    uint32_t i = mbi->mmap_addr;
    char buf[100];
    while (i < mbi->mmap_addr + mbi->mmap_length) {
        memset(buf, 0, 100);
        multiboot_memory_map_t *me = (multiboot_memory_map_t*) i;
        puts("Starts at: 0x");
        itoa(((uint64_t)me->addr_high << 32) | me->addr_low, buf, 10);
        puts(buf);
        puts(" | ");
        itoa(me->len_low, buf, 10);
        puts(buf);
        puts(" bytes long | Size: 0x");
        itoa(me->size, buf, 16);
        puts(buf);
        puts(" | Type: ");
        itoa(me->type, buf, 10);
        puts(buf);
        puts("\n");

        if (me->type == 1) {
            total_bytes_ram += me->len_low;
        }

        i += me->size + sizeof(uint32_t);
    }

    /* FIXME: CURRENTLY WRONG??? */
    puts("Kernel size: ");
    itoa((mbi->mem_upper - mbi->mem_lower + KERNEL_START) / 1000, buf, 10);
    puts(buf);
    puts("KB\n");
    puts("Total Ram: ");
    itoa((total_bytes_ram) / 1000000, buf, 10); /* converts to MB */
    puts(buf);
    puts("MB\n");
    curr_free_mem = mbi->mem_upper - mbi->mem_lower + KERNEL_START;
}

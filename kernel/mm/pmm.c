#include <stdint.h>

#include "../stdlib/util.h"
#include "../stdlib/bitmap/bitmap.h"
#include "../drivers/tty.h"
#include "paging.h"
#include "pmm.h"
#include "../multiboot.h"

static uint32_t used_memory = 0;
static uint32_t reserved_memory = 0;

/* I stg I am going schitzo */

/* Length of page frame will always be 4096 bytes (4KiB) (0x1000) */
static uint8_t bitmap[SIZE_OF_BITMAP] = {0};
/* 
 *   76543210
 * 1 00010100
 * 2 00001001
 *
 * Algo is byte * WORD_LENGTH + bit
 * */

uint32_t
pmm_get_used_memory()
{
    return used_memory;
}

uint32_t
pmm_get_reserved_memory()
{
    return reserved_memory;
}

void
pmm_set_frame(uint32_t frame)
{
    set_bit(bitmap, frame);
}

void
pmm_clear_frame(uint32_t frame)
{
    clear_bit(bitmap, frame);
}

uint8_t
pmm_get_frame(uint32_t frame)
{
    return get_bit(bitmap, frame);
}

uint32_t
pmm_alloc_frame()
{
    uint32_t frame = find_first_free_bit(bitmap, SIZE_OF_BITMAP);
    // kprintf("FOUND FRAME: 0x%x\n", frame);
    pmm_set_frame(frame);

    used_memory += PAGE_FRAME_SIZE;

    // kprintf("RETURNING ADDR: 0x%x\n", frame * 0x1000);
    return frame * 0x1000;
}

void
pmm_init(multiboot_info_t* mbi)
{
    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
    uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

    /* Set all entries as taken */
    memset(bitmap, 0xFF, SIZE_OF_BITMAP);

    while ((uint32_t)mmap < mmap_end) {
        uint64_t base_addr = ((uint64_t)mmap->addr_high << 32) | mmap->addr_low;
        uint64_t length = ((uint64_t)mmap->len_high << 32) | mmap->len_low;

        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            /* FIXME: Is this a good idea?? */
            base_addr = ALIGN_DOWN(base_addr);
            length = ALIGN_DOWN(length);

            /* Do not go past 4 GiB barrier */
            if ((base_addr / PAGE_SIZE) >= 0x100000000) {
                break;
            }

            for (uint32_t i = 0; i < (length / PAGE_SIZE); i++) {
                pmm_clear_frame((base_addr / PAGE_SIZE) + i);
            }

        } else if (mmap->type == MULTIBOOT_MEMORY_RESERVED) {
            /* Do nothing */
        } else {
            /* Do nothing */
        }

        mmap = (multiboot_memory_map_t*)((uint32_t)mmap + mmap->size + sizeof(uint32_t));

    }

    /* We do not want to be able to allocate address 0 */
    pmm_set_frame(0x0);
}

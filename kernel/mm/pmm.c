#include <stdint.h>

#include "../stdlib/util.h"
#include "../stdlib/bitmap/bitmap.h"
#include "../drivers/tty.h"
#include "paging.h"
#include "pmm.h"

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
    pmm_set_frame(frame);

    used_memory += PAGE_FRAME_SIZE;

    return frame * 0x1000;
}

/* FIXME: Change this to identity mapping the kernel */
void
pmm_reserve_memory()
{
    /* Kernel starts at 0x100000, so we need to reserve
     * up to 0x100000 plus however big the kernel is.
     * CURRENTLY the loop iterates over pages UP TO 0x100000
     * */
    for (uint32_t page = 1; page < ((KERNEL_START) / PAGE_FRAME_SIZE); page++) {
        pmm_set_frame(page);
        reserved_memory += PAGE_FRAME_SIZE;
    }

    /* What do I need to reserve all non usable memory? 
     * 1. Number of reserved sections 
     * 2. Address of each reserved section 
     * 3. Length of reserved section 
     *
     * I will have a nested for loop, outsidse will be number
     * of sections inside will start at the address of the section
     * and run for the length of the section 
     *
     * Based on the above implementation idea I think the easiest way
     * to store the info would be an array of structs */
}

void
pmm_init()
{
}

#include <stdint.h>

#include "../stdlib/util.h"
#include "../drivers/tty.h"
#include "paging.h"
#include "pmm.h"

uint32_t used_memory = 0;
uint32_t reserved_memory = 0;

/* I stg I am going schitzo */

/* FIXME: SET THIS */
uint32_t KERNEL_SIZE = 1; 

/* Length of page frame will always be 4096 bytes (4KiB) (0x1000) */
uint8_t bitmap[SIZE_OF_BITMAP] = {0};
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
    bitmap[WORD_OFFSET(frame)] |= (1 << BIT_OFFSET(frame));
}

void
pmm_clear_frame(uint32_t frame)
{
    bitmap[WORD_OFFSET(frame)] &= ~(1 << BIT_OFFSET(frame));
}

uint8_t
pmm_get_frame(uint32_t frame)
{
    uint8_t ret = bitmap[WORD_OFFSET(frame)] & (1 << BIT_OFFSET(frame));
    return ret != 0;
}

uint32_t
pmm_find_free_frames(uint32_t num_frames)
{
    uint32_t start_bit  = 0;
    uint32_t free_frames = 0;

    /* Goes through each individual bit */
    for (uint32_t i = 0; i < SIZE_OF_BITMAP * 8; i++) {
        /* The frame is already taken */
        if (pmm_get_frame(i)) {
            start_bit = i + 1;
            free_frames = 0;
            continue;
        }

        free_frames++;

        /* Found it! */
        if (free_frames == num_frames) {
            return (WORD_OFFSET(start_bit) << 3) + BIT_OFFSET(start_bit);
        }
    }

    return 0;
}

/* Returns the first free page frame */
uint32_t
pmm_find_free_frame()
{
    for (uint32_t i = 0; i < SIZE_OF_BITMAP; i++) {
        uint8_t byte = bitmap[i];

        /* Move on if no 0 bits in byte */
        if (byte == 0xFF)
            continue;

        /* Get rightmost 0 bit (free frame) */
        uint8_t offset = __builtin_ctz(~byte);

        /* Finds the frame number */
        return i * WORD_LENGTH + offset;
    }

    return 0;
}

void*
pmm_alloc_frame()
{
    uint32_t frame = pmm_find_free_frame();
    pmm_set_frame(frame);

    used_memory += PAGE_FRAME_SIZE;

    return (void*)(frame * 0x1000);
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

#include <stdint.h>

#include "../stdlib/util.h"
#include "../drivers/tty.h"

/* I stg I am going schitzo */

/*
 * 4 GiB / 4KiB = 1 MiB (number of pages)
 * 1 MiB / 8 bytes = 131072 bytes = 0x20000 (sizeof bitmap)
 */
#define MAX_NUM_OF_PAGE_FRAMES 0x100000
/* NOTE: Switching this to 32 may improve performance */
#define WORD_LENGTH            0x8
#define SIZE_OF_BITMAP         MAX_NUM_OF_PAGE_FRAMES / WORD_LENGTH
#define KERNEL_START           0x100000  /* 1 MiB */
#define PAGE_FRAME_SIZE        0x1000    /* 4 KiB */
#define RESERVED_FOR_KERNEL    0x6400000 /* 100 MiB */

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

uint32_t used_memory = 0;
uint32_t reserved_memory = 0;

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
pmm_alloc_page_frame(uint32_t frame)
{
    bitmap[frame / WORD_LENGTH] |= (1 << (frame % WORD_LENGTH));
}

void
pmm_free_page_frame(uint32_t frame)
{
    bitmap[frame / WORD_LENGTH] &= ~(1 << (frame % WORD_LENGTH));
}

uint8_t
pmm_get_page_frame(uint32_t frame)
{
    uint8_t ret = bitmap[frame / WORD_LENGTH] & (1 << (frame % WORD_LENGTH));
    return ret != 0;
}

/* Returns the first free page frame */
uint32_t
pmm_find_free_page_frame()
{
    for (uint32_t i = 0; i < SIZE_OF_BITMAP; i++) {
        uint8_t byte = bitmap[i];

        /* Move on if no 0 bits in byte */
        if (byte == 255)
            continue;

        /* Get rightmost 0 bit (free page) */
        uint8_t offset = __builtin_ctz(~byte);

        /* Finds the page number */
        return i * WORD_LENGTH + offset;
    }
    return 0;
}

/* Returns the address of the page frame */
void*
pmm_request_page_frame()
{
    uint32_t page_number = pmm_find_free_page_frame();
    pmm_alloc_page_frame(page_number);

    used_memory += 4096;

    return (void*)(page_number * 0x1000);
}

void
pmm_reserve_memory()
{
    /* Kernel starts at 0x100000, so we need to reserve
     * up to 0x100000 plus however big the kernel is.
     * CURRENTLY the loop iterates over pages UP TO 0x100000
     * */
    for (uint32_t page = 0; page < ((KERNEL_START) / PAGE_FRAME_SIZE); page++) {
        pmm_alloc_page_frame(page);
        reserved_memory += 4096;
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
    pmm_reserve_memory();
}

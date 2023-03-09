#include <stdint.h>

#include "../stdlib/util.h"
#include "../drivers/tty.h"

/*
 * 4 GiB / 4KiB = 1 MiB
 * 1 MiB / 8 bytes = 131072 bytes = 0x20000
 */
#define MAX_NUM_OF_PAGES 0x20000
/* NOTE: Switching this to 32 may improve performance */
#define WORD_LENGTH      0x8
#define KERNEL_START     0x100000 /* 1 MiB */
#define PAGE_SIZE        0x1000   /* 4 KiB */

uint32_t KERNEL_SIZE     = 1; 

/* Length of page frame will always be 4096 bytes (4KiB) (0x1000) */
uint8_t bitmap[MAX_NUM_OF_PAGES];
/* 
 *   76543210
 * 1 00000000
 * 2 00000000
 *
 * Algo is byte * 8 + bit
 * */

void
set_page(uint8_t bit)
{
    bitmap[bit / WORD_LENGTH] |= (1 << (bit % WORD_LENGTH));
}

void
clear_page(uint8_t bit)
{
    bitmap[bit / WORD_LENGTH] &= ~(1 << (bit % WORD_LENGTH));
}

void
reserve_memory()
{
    /* Kernel starts at 0x100000, so we need to reserve
     * up to 0x100000 plus however big the kernel is.
     * CURRENTLY the loop iterates over pages UP TO 0x100000 */
    for (uint32_t page = 0; page < (KERNEL_START / PAGE_SIZE); page++) {
        set_page(page);
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

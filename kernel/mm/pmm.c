#include <stdint.h>

#include "../stdlib/util.h"
#include "../drivers/tty.h"

/* I stg I am going schitzo */

/*
 * 4 GiB / 4KiB = 1 MiB pages
 * 1 MiB / 8 bytes = 131072 bytes = 0x20000
 */
#define MAX_NUM_OF_PAGES 0x100000
/* NOTE: Switching this to 32 may improve performance */
#define WORD_LENGTH      0x8
#define SIZE_OF_BITMAP   MAX_NUM_OF_PAGES / WORD_LENGTH
#define KERNEL_START     0x100000 /* 1 MiB */
#define PAGE_SIZE        0x1000   /* 4 KiB */

/* FIXME: SET THIS */
uint32_t KERNEL_SIZE = 1; 

/* Length of page frame will always be 4096 bytes (4KiB) (0x1000) */
uint8_t bitmap[SIZE_OF_BITMAP];
/* 
 *   76543210
 * 1 00010100
 * 2 00001001
 *
 * Algo is byte * WORD_LENGTH + bit
 * */

void
pmm_set_page(uint32_t page)
{
    bitmap[page / WORD_LENGTH] |= (1 << (page % WORD_LENGTH));
}

void
pmm_free_page(uint32_t page)
{
    bitmap[page / WORD_LENGTH] &= ~(1 << (page % WORD_LENGTH));
}

uint8_t
pmm_get_page(uint32_t page)
{
    uint8_t ret = bitmap[page / WORD_LENGTH] & (1 << (page % WORD_LENGTH));
    return ret != 0;
}

/* Returns the first free page */
uint32_t
pmm_find_free_page()
{
    for (uint32_t i = 0; i < SIZE_OF_BITMAP; i++) {
        uint32_t byte = bitmap[i];

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

/* Returns the address of the page */
uint32_t
pmm_request_page()
{
    uint32_t page_number = pmm_find_free_page();
    pmm_set_page(page_number);
    return page_number * 0x1000;
}

void
pmm_reserve_memory()
{
    /* Kernel starts at 0x100000, so we need to reserve
     * up to 0x100000 plus however big the kernel is.
     * CURRENTLY the loop iterates over pages UP TO 0x100000 */
    for (uint32_t page = 0; page < (KERNEL_START / PAGE_SIZE); page++) {
        pmm_set_page(page);
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
    memset(bitmap, 0, SIZE_OF_BITMAP);
    pmm_reserve_memory();
}

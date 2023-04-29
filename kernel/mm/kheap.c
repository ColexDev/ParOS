#include <stdint.h>

#include "paging.h"
#include "pmm.h"
#include "kheap.h"
#include "../stdlib/util.h"
#include "../stdlib/bitmap/bitmap.h"

/* Size of bitmap: 
    * 8 byte block size means 0x20000 blocks per 1 MiB of heap.
    * That is 16 KiB or 4 pages worth */

/* NOTES:
* So I have to keep track of what virtual address I am currently on for the next address.
* What virtual address should I start at? For the kernel it has to be whatever the physical address
* is due to it being identity mapped? WAIT. I think in the kernel I can just grab the next avaliable
* physical address, and thats going to be the virtual address?
*
* nonono, I need to setup higher half kernel first, then I need to set the kernel heap to a
* higher part of the higher half mapping, it will be hardcoded ex: 0xf0000000 */

/* Implementation 
 * NOTE: This is VERY BAD and may lead to lots of internal fragmentation
 * but I honestly do not care at the moment, I can change it later if I want.
 *
    * Create a variable sized bitmap that is one block per bit. 
    *
    * If there is a call to increase the heap, allocate a new page
    * and increase the size of bitmap by that amount. 
    *
    * When malloc is called, determine the number of blocks needed 
    * to fufill the request, return the VIRTUAL address to the first
    * block. The Virtual address can be calculated by taking the start
    * of the heap and adding onto it.
    *
    * Have one block be the header with a magic number (4 bytes), and the
    * size of the block (4 bytes) including the header.
    *
    * When freeing, get the size of the allocation from the header
    * and free from the starting address for the size of the allocation */

/* Move this off the stack eventually */
static uint8_t bitmap[HEAP_START_SIZE / BLOCK_SIZE / 8] = {0};

/* When testing this, my first allocation should return 
 * 0xF0000008 since the header takes up the first block */
void*
kmalloc(uint32_t size)
{
    uint32_t num_blocks   = size / BLOCK_SIZE;
    uint32_t total_blocks = HEAP_START_SIZE / BLOCK_SIZE;
    uint32_t free_frames  = 0;
    uint32_t start_bit    = 0;
    uint32_t start_block  = 0;
    uint32_t header_block = 0;
    uint32_t* start_addr  = 0;

    struct block_header header;

    if (size % BLOCK_SIZE)
        num_blocks = (size / BLOCK_SIZE) + 1;

    header.size = num_blocks;
    header.magic = HEAP_MAGIC;

    /* FIXME: Use the bitmap function here */
    for (uint32_t i = 0; i < total_blocks; i++) {
        /* The frame is already taken */
        if (get_bit(bitmap, i)) {
            start_bit = i + 1;
            free_frames = 0;
            continue;
        }

        free_frames++;

        /* + 1 accounts for the block needed for the header */
        if (free_frames == num_blocks + 1) {
            start_block = (WORD_OFFSET(start_bit) << 3) + BIT_OFFSET(start_bit);
        }
    }

    /* Set the frame(s) as used */
    for (uint32_t block = 0; block < num_blocks + 1; block++) {
        set_bit(bitmap, start_block + block);
    }

    /* Calculate the starting address of the block of memory, this is
     * where the header resides, the address returned is after that */
    start_addr = (uint32_t*)((start_block * BLOCK_SIZE) + HEAP_START_ADDR);

    /* Set header */
    memcpy(start_addr, &header, sizeof(struct block_header));

    return (void*)((uint32_t)start_addr + sizeof(struct block_header));
}

void
kfree(void* ptr)
{
    // kprintf("Before free bitmap: %d\n", bitmap[0]);
    struct block_header* header;
    void* full_ptr = ptr - sizeof(struct block_header);
    uint32_t start_block;

    header = full_ptr;

    start_block = (uint32_t)(full_ptr - HEAP_START_ADDR) / BLOCK_SIZE;

    /* What should I do here??? */
    if (header->magic != HEAP_MAGIC)
        return;

    for (uint32_t block = 0; block < header->size + 1; block++) {
        clear_bit(bitmap, start_block + block);
    }

    // kprintf("addr of ptr->0x%x\n", ptr);
    // kprintf("Start block->%d\n", start_block);
    // kprintf("Magic->0x%x\n", header->magic);
    // kprintf("Size->%d\n", header->size);
    // kprintf("After free bitmap: %d\n", bitmap[0]);
}

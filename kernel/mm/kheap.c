#include <stdint.h>

#include "paging.h"
#include "pmm.h"

#define BLOCK_SIZE 32 /* bytes */
#define NUM_BLOCKS_PER_PAGE PAGE_SIZE / BLOCK_SIZE

/* Size of bitmap: 
    * For ever 32 bytes (256 bits) we store we need 1 bit,
    * so for every 256 bytes you need 1 byte.
    * If my math is correct this means I need 1 page (4KiB) of bitmap
    * per 1MiB of heap 
    *
    * 256 bytes -> 1 byte
    * 0x100000 (256 * 0x1000) bytes -> 0x1000 (1 * 0x1000) bytes */

/* NOTES:
* So I have to keep track of what virtual address I am currently on for the next address.
* What virtual address should I start at? For the kernel it has to be whatever the physical address
* is due to it being identity mapped? WAIT. I think in the kernel I can just grab the next avaliable
* physical address, and thats going to be the virtual address?
*
* nonono, I need to setup higher half kernel first, then I need to set the kernel heap to a
* higher part of the higher half mapping, it will be hardcoded ex: 0xf0000000 */

/* Implementation 
 * NOTE: This is VERY BAD and will lead to lots of internal fragmentation
 * but I honestly do not care at the moment, I can change it later if I want
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
    * Have one block be the header with a magic number(4 bytes), and the
    * size of the block (4 bytes) including the header.
    *
    * When freeing, get the size of the allocation from the header
    * and free from the starting address for the size of the allocation */

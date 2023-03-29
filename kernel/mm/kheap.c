#include <stdint.h>

#include "paging.h"
#include "pmm.h"

#define BLOCK_SIZE 8
#define NUM_BLOCKS_PER_PAGE PAGE_SIZE / BLOCK_SIZE

/* NOTES:
* So I have to keep track of what virtual address I am currently on for the next address.
* What virtual address should I start at? For the kernel it has to be whatever the physical address
* is due to it being identity mapped? WAIT. I think in the kernel I can just grab the next avaliable
* physical address, and thats going to be the virtual address?
*
* nonono, I need to setup higher half kernel first, then I need to set the kernel heap to a
* higher part of the higher half mapping, it will be hardcoded ex: 0xf0000000 */

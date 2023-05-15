#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>

#define BLOCK_SIZE 8 /* bytes */
#define NUM_BLOCKS_PER_PAGE PAGE_SIZE / BLOCK_SIZE
#define HEAP_START_SIZE 0x100000
/* Since page tables start at 0xFFC00000, this leaves
 * 264 MiB for the heap */
#define HEAP_START_ADDR 0xF0000000
#define HEAP_END_ADDR   0xFFBFFFFF
#define HEAP_MAGIC      0x123ABCFE

struct __attribute__((packed)) block_header {
    uint32_t magic;
    uint32_t size;
};

void* kmalloc(uint32_t size);
void* krealloc(void* ptr, uint32_t size);
void kfree(void* ptr);
uint32_t ksize(void* ptr);

#endif /* #ifndef KHEAP_H */

#ifndef KHEAP_H
#define KHEAP_H

#define HEAP_START_SIZE 0x100000
/* Since page tables start at 0xFFC00000, this leaves
 * 264 MiB for the heap */
#define HEAP_START_ADDR 0xF0000000
#define HEAP_END_ADDR   0xFFBFFFFF

#endif /* #ifndef KHEAP_H */

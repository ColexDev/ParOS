#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096
#define WORD_LENGTH 8

#define WORD_OFFSET(b) ((b) / WORD_LENGTH)
#define BIT_OFFSET(b)  ((b) % WORD_LENGTH)

void pmm_init(void);
void* pmm_alloc(uint64_t num_frames);
void pmm_free(void* addr, uint64_t num_frames);

#endif /* PMM_H */

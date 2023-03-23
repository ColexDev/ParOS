#ifndef PMM_H
#define PMM_H

#include <stdint.h>

/*
 * 4 GiB / 4KiB = 1 MiB (number of pages)
 * 1 MiB / 8 bytes = 131072 bytes = 0x20000 (sizeof bitmap)
 */
#define MAX_NUM_OF_PAGE_FRAMES 0x100000  /* 1 MiB */
/* NOTE: Switching this to 32 may improve performance */
#define WORD_LENGTH            0x8
#define SIZE_OF_BITMAP         MAX_NUM_OF_PAGE_FRAMES / WORD_LENGTH
#define KERNEL_START           0x100000  /* 1 MiB */
#define PAGE_FRAME_SIZE        0x1000    /* 4 KiB */
#define RESERVED_FOR_KERNEL    0x6400000 /* 100 MiB */

#define WORD_OFFSET(b) ((b) / WORD_LENGTH)
#define BIT_OFFSET(b)  ((b) % WORD_LENGTH)

void pmm_init();
uint32_t pmm_get_used_memory();
uint32_t pmm_get_reserved_memory();

void pmm_set_frame(uint32_t frame);
void pmm_clear_frame(uint32_t frame);
uint32_t pmm_find_free_frame();
void* pmm_alloc_frame();

#endif /* #ifndef PMM_H */

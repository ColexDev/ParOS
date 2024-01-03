#include <stdint.h>

#include <klibc/string/string.h>
#include <mem/memmap/memmap.h>
#include <io/printf.h>

#include "pmm.h"

static uint8_t* bitmap;
static uint64_t bitmap_size;

void
pmm_init(void)
{
    uint64_t max_addr = memmap_get_highest_usable_address();
    uint64_t num_page_frames = ALIGN_UP(max_addr, PAGE_SIZE) / PAGE_SIZE;
    bitmap_size = num_page_frames / 8;

    kprintf("Max Addr: 0x%llx | %lld\n", max_addr, max_addr);
    kprintf("Page Frames: %lld\n", num_page_frames);
    kprintf("Bitmap Size: %lld\n", bitmap_size);
}

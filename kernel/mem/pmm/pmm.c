#include <stdint.h>

#include <klibc/string/string.h>
#include <mem/memmap/memmap.h>
#include <io/printf.h>

#include "pmm.h"

static uint8_t* bitmap;
static uint64_t bitmap_size;

/* So when I start on vmm I need to think about stuff:
 * Paging is enabled and the HHDM exists.
 * So the virtual addresses for everything besides the kernel (I think)
 * are just the hhdm offset + phys addr
 * And then the kernel is just the starting phys address
 * mapped to 0xffffffff80000000.
 * So just set up my own HHDM of sorts and map the kernel to the same
 * spot in virtual memory
 */

void
pmm_set_frame(uint64_t n)
{
    bitmap[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void
pmm_clear_frame(uint64_t n)
{
    bitmap[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n));
}

void
pmm_init(void)
{
    uint64_t max_addr = memmap_get_highest_usable_address();
    uint64_t num_page_frames = ALIGN_UP(max_addr, PAGE_SIZE) / PAGE_SIZE;
    bitmap_size = num_page_frames / 8;

    struct memmap_entry curr_entry;

    /* Find space for the bitmap */
    for (uint64_t i = 0; i < memmap_get_num_entries(); i++) {
        curr_entry = memmap_get_entry(i);
        if (curr_entry.type != MEMMAP_USABLE) {
            continue;
        }

        if (curr_entry.length >= bitmap_size) {
            /* FIXME: ADD HHDM OFFSET TO THE BASE */
            bitmap = (uint8_t*)(curr_entry.base);
            break;
        }
    }

    /* Set all to 1 (non-free) */
    memset(bitmap, 0xFF, bitmap_size);

    /* Clear usable entries */
    for (uint64_t i = 0; i < memmap_get_num_entries(); i++) {
        curr_entry = memmap_get_entry(i);
        if (curr_entry.type != MEMMAP_USABLE) {
            continue;
        }
        
        for (uint64_t j = curr_entry.base / PAGE_SIZE; j < (curr_entry.base / curr_entry.length) / PAGE_SIZE; j += PAGE_SIZE) {
            pmm_clear_frame(j);
        }
    }

    /* Now we need to re-set the frames that the bitmap occupies */
    /* FIXME: This needs to be bitmap without HHDM */
    for (uint64_t i = (uint64_t)bitmap / PAGE_SIZE; i < ((uint64_t)bitmap - bitmap_size) / PAGE_SIZE; i += PAGE_SIZE) {
        pmm_set_frame(i);
    }

    kprintf("Max Addr: 0x%llx | %lld\n", max_addr, max_addr);
    kprintf("Page Frames: %lld\n", num_page_frames);
    kprintf("Bitmap Size: 0x%llxB\n", bitmap_size);
    kprintf("Bitmap Start Addr: 0x%llx\n", (uint64_t)bitmap);
    kprintf("Bitmap End Addr: 0x%llx\n", (uint64_t)bitmap + bitmap_size);
}

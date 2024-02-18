#include <stdint.h>

#include <klibc/string/string.h>
#include <mem/memmap/memmap.h>
#include <io/printf.h>
#include <bl/bl.h>

#include "pmm.h"

static uint8_t* bitmap;
static uint64_t bitmap_size;        /* Size of bitmap in BYTES in memory */

static uint64_t free_pages     = 0; /* Free usable pages */
static uint64_t used_pages     = 0; /* Usable used pages (from pmm_alloc) */
static uint64_t taken_pages    = 0; /* Total taken pages in bitmap */
static uint64_t usable_pages   = 0; /* Total usable pages (probably RAM) */
static uint64_t unusable_pages = 0; /* Total unusable pages (from memory holes) */
static uint64_t reserved_pages = 0; /* Total reserved pages (kernel, ACPI, etc) */

/* So when I start on vmm I need to think about stuff:
 * Paging is enabled and the HHDM exists.
 * So the virtual addresses for everything besides the kernel (I think)
 * are just the HHDM offset + phys addr
 * And then the kernel is just the starting phys address
 * mapped to 0xffffffff80000000.
 * So just set up my own HHDM of sorts and map the kernel to the same
 * spot in virtual memory */

uint64_t
pmm_get_used_pages(void)
{
    return used_pages;
}

uint64_t
pmm_get_free_pages(void)
{
    return free_pages;
}

static void
pmm_set_frame(uint64_t frame)
{
    taken_pages++;
    free_pages--;
    bitmap[WORD_OFFSET(frame)] |= (1 << BIT_OFFSET(frame));
}

static void
pmm_clear_frame(uint64_t frame)
{
    taken_pages--;
    free_pages++;
    bitmap[WORD_OFFSET(frame)] &= ~(1 << BIT_OFFSET(frame));
}

static uint8_t
pmm_test_frame(uint64_t frame)
{
    return (bitmap[WORD_OFFSET(frame)] & (1 << BIT_OFFSET(frame))) != 0;
}

/* Return starting frame of the set */
static uint64_t
pmm_find_free_frames(uint64_t num_frames)
{
    uint32_t start_frame = 0;
    uint32_t free_frames = 0;

    /* Goes through each individual bit */
    for (uint64_t i = 0; i < bitmap_size * WORD_LENGTH; i++) {
        /* The bit is already taken */
        if (pmm_test_frame(i)) {
            start_frame = i + 1;
            free_frames = 0;
            continue;
        }

        free_frames++;

        /* Found it! */
        if (free_frames == num_frames) {
            // return (WORD_OFFSET(start_bit) * WORD_LENGTH) + BIT_OFFSET(start_bit);
            return start_frame;
        }
    }

    kprintf("start_frame: %lld\n", start_frame);
    kprintf("free_frames: %lld\n", free_frames);
    return 0;
}

/* Return address of start */
void*
pmm_alloc(uint64_t num_frames)
{
    uint64_t start_frame;
    start_frame = pmm_find_free_frames(num_frames);

    for (uint64_t i = start_frame; i < start_frame + num_frames; i++) {
        used_pages++;
        pmm_set_frame(i);
    }

    return (void*)(start_frame * PAGE_SIZE);
}

void
pmm_free(void* start_addr, uint64_t num_frames)
{
    uint64_t start_frame = (uint64_t)start_addr / PAGE_SIZE;

    for (uint64_t i = start_frame; i < start_frame + num_frames; i++) {
        used_pages--;
        pmm_clear_frame(i);
    }
}

/* NOTE:
 * bitmap_size_frames and total_page_frames both exist because there are memory
 * holes. bitmap_size_frames does NOT account for these holes, therefore the bitmap
 * has a bunch of spaces that will always be counted as non-free as the addresses are
 * not even used/accessible. total_page_frames represents the number of page frames 
 * that actually exist. This includes reserved and all types that may not be usable,
 * but they actually exist. bitmap_size_frames should not be used besides determining
 * the size of the bitmap. total_page_frames should always be used when determining 
 * how many pages frames are in the bitmap (in total).
 */
void
pmm_init(void)
{
    uint64_t max_usable_addr = memmap_get_highest_usable_address();

    /* This is how many frames the bitmap is CAPABLE of holding */
    uint64_t bitmap_num_frames = ALIGN_UP(max_usable_addr, PAGE_SIZE) / PAGE_SIZE;

    /* This is how many BYTES the bitmap takes up in memory */
    bitmap_size = bitmap_num_frames / WORD_LENGTH;

    struct memmap_entry curr_entry;

    /* Find space for the bitmap */
    for (uint64_t i = 0; i < memmap_get_num_entries(); i++) {
        curr_entry = memmap_get_entry(i);
        if (curr_entry.type != MEMMAP_USABLE) {
            continue;
        }

        if (curr_entry.length >= bitmap_size) {
            bitmap = (uint8_t*)(curr_entry.base + bl_get_hhdm_offset());
            break;
        }
    }

    /* Set all to 1 (non-free) */
    memset(bitmap, 0xFF, bitmap_size);
    taken_pages = bitmap_size * WORD_LENGTH;

    /* NOTE: Num entries - 1 because qemu has weird last reserved entry that is massive */
    uint64_t total_page_frames = 0;
    for (uint64_t i = 0; i < memmap_get_num_entries() - 1; i++) {
        curr_entry = memmap_get_entry(i);

        uint64_t aligned_length = ALIGN_UP(curr_entry.length, PAGE_SIZE);

        switch (curr_entry.type) {
        case MEMMAP_USABLE:
            // kprintf("PMM_CLEAR: Clearing %lld frames from 0x%llx to 0x%llx\n",
            //         (aligned_length / PAGE_SIZE), curr_entry.base, curr_entry.base + curr_entry.length);
            /* FIXME: Figure out the <= vs < stuff. see value of (curr_entry.base + aligned_length) */
            for (uint64_t j = curr_entry.base / PAGE_SIZE; j < (curr_entry.base + aligned_length) / PAGE_SIZE; j++) {
                pmm_clear_frame(j);
            }

            usable_pages      += aligned_length / PAGE_SIZE;
            total_page_frames += aligned_length / PAGE_SIZE;
            break;
        case MEMMAP_KERNEL_AND_MODULES:     /* Fallthrough */
        case MEMMAP_RESERVED:               /* Fallthrough */
        case MEMMAP_ACPI_RECLAIMABLE:       /* Fallthrough */
        case MEMMAP_ACPI_NVS:               /* Fallthrough */
        case MEMMAP_BAD_MEMORY:             /* Fallthrough */
        case MEMMAP_BOOTLOADER_RECLAIMABLE: /* Fallthrough */
        case MEMMAP_FRAMEBUFFER:
            reserved_pages    += aligned_length / PAGE_SIZE;
            total_page_frames += aligned_length / PAGE_SIZE;
            break;
        }
    }

    /* Now we need to re-set the frames that the bitmap occupies */
    uint64_t bitmap_phys = (uint64_t)bitmap - bl_get_hhdm_offset();

    /* NOTE: It is < and not <= because the upper address of a memory map entry
    * is EXCLUSIVE and not INCLUSIVE */
    for (uint64_t i = bitmap_phys / PAGE_SIZE; i < (bitmap_phys + bitmap_size) / PAGE_SIZE; i++) {
        pmm_set_frame(i);
    }

    unusable_pages = (max_usable_addr / PAGE_SIZE) - (reserved_pages + usable_pages);

    used_pages = taken_pages - unusable_pages - reserved_pages - (bitmap_size / PAGE_SIZE);

    kprintf("Highest Usable Addr: 0x%llx\n", max_usable_addr);
    kprintf("Bitmap Size: 0x%llxB | %lld frames\n", bitmap_size, bitmap_size / PAGE_SIZE);
    kprintf("Bitmap Phys Start Addr: 0x%llx\n", (uint64_t)bitmap_phys);
    kprintf("Bitmap Phys End Addr: 0x%llx\n", (uint64_t)bitmap_phys + bitmap_size);
    kprintf("Bitmap Virt Start Addr: 0x%llx\n", (uint64_t)bitmap);
    kprintf("Bitmap Virt End Addr: 0x%llx\n", (uint64_t)bitmap + bitmap_size);
    kprintf("free pages: %lld\tused/taken pages: %lld\nreserved pages: %lld\tusable pages: %lld\n", 
            free_pages,        taken_pages,       reserved_pages,       usable_pages);
    kprintf("Total Page Frames from memmap: %lld\n", total_page_frames);
    kprintf("UNUSABLE Page Frames: %lld\n", unusable_pages);
    kprintf("Used/taken (including reserved and bitmap) Page Frames: %lld\n", taken_pages - unusable_pages);
    kprintf("ACTUALLY USED (from pmm_alloc) Page Frames: %lld\n", 
            used_pages);

    // kprintf("used_pages: %lld\n", used_pages);
    // kprintf("free_pages: %lld\n", free_pages);
    // kprintf("Allocing 5 frames\n");
    // void* test = pmm_alloc(5);
    // kprintf("used_pages after alloc: %lld\n", used_pages);
    // kprintf("free_pages: %lld\n", free_pages);
    // kprintf("Alloced 5 frames starting at 0x%llx\n", test);
    // kprintf("Freeing 5 frames\n");
    // pmm_free(test, 5);
    // kprintf("Allocing 12 frames\n");
    // void* test2 = pmm_alloc(12);
    // kprintf("used_pages after alloc: %lld\n", used_pages);
    // kprintf("free_pages: %lld\n", free_pages);
    // kprintf("Alloced 12 frames starting at 0x%llx\n", test2);
}

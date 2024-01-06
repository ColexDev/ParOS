#include <stdint.h>

#include <klibc/string/string.h>
#include <mem/memmap/memmap.h>
#include <io/printf.h>
#include <bl/bl.h>

#include "pmm.h"

static uint8_t* bitmap;
static uint64_t bitmap_size;

static uint64_t free_pages     = 0;
static uint64_t used_pages     = 0;
static uint64_t usable_pages   = 0;
static uint64_t unusable_pages = 0;
static uint64_t reserved_pages = 0;

/* So when I start on vmm I need to think about stuff:
 * Paging is enabled and the HHDM exists.
 * So the virtual addresses for everything besides the kernel (I think)
 * are just the hhdm offset + phys addr
 * And then the kernel is just the starting phys address
 * mapped to 0xffffffff80000000.
 * So just set up my own HHDM of sorts and map the kernel to the same
 * spot in virtual memory */

static void
pmm_set_frame(uint64_t frame)
{
    used_pages++;
    free_pages--;
    bitmap[WORD_OFFSET(frame)] |= (1 << BIT_OFFSET(frame));
}

static void
pmm_clear_frame(uint64_t frame)
{
    used_pages--;
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
        pmm_set_frame(i);
    }

    return (void*)(start_frame * PAGE_SIZE);
}

void
pmm_free(void* addr, uint64_t num_frames)
{
    uint64_t start_frame = (uint64_t)addr / PAGE_SIZE;

    for (uint64_t i = start_frame; i < start_frame + num_frames; i++) {
        pmm_clear_frame(i);
    }

}

void
pmm_init(void)
{
    uint64_t max_usable_addr = memmap_get_highest_usable_address();
    uint64_t num_page_frames = ALIGN_UP(max_usable_addr, PAGE_SIZE) / PAGE_SIZE;

    bitmap_size = num_page_frames / 8;

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
    used_pages = bitmap_size * WORD_LENGTH;
    kprintf("===ORIGINAL USED PAGES: %lld===\n", used_pages);

    /* NOTE: Num entries - 1 because qemu has weird last reserved entry that is massive */
    uint64_t total_page_frames = 0;
    for (uint64_t i = 0; i < memmap_get_num_entries() - 1; i++) {
        curr_entry = memmap_get_entry(i);

        switch (curr_entry.type) {
        case MEMMAP_USABLE:
            // kprintf("PMM_CLEAR: Clearing %lld frames from 0x%llx to 0x%llx\n",
            //         (curr_entry.length / PAGE_SIZE), curr_entry.base, curr_entry.base + curr_entry.length);
            /* FIXME: Figure out the <= vs < stuff. see value of (curr_entry.base + curr_entry.length) */
            for (uint64_t j = curr_entry.base / PAGE_SIZE; j < (curr_entry.base + curr_entry.length) / PAGE_SIZE; j++) {
                pmm_clear_frame(j);
            }

            usable_pages += curr_entry.length / PAGE_SIZE;
            total_page_frames += curr_entry.length / PAGE_SIZE;
            kprintf("%d: Current entry length: %lld\n", i, curr_entry.length);
            kprintf("%d: Num frames: %lld\n", i, curr_entry.length / PAGE_SIZE);
            kprintf("%d: Usable pages: %lld\n", i, usable_pages);
            break;
        case MEMMAP_KERNEL_AND_MODULES:
        case MEMMAP_RESERVED:              /* Fallthrough */
        case MEMMAP_ACPI_RECLAIMABLE:
        case MEMMAP_ACPI_NVS:
        case MEMMAP_BAD_MEMORY:
        case MEMMAP_BOOTLOADER_RECLAIMABLE:
        case MEMMAP_FRAMEBUFFER:
            total_page_frames += curr_entry.length / PAGE_SIZE;
            reserved_pages += curr_entry.length / PAGE_SIZE;
            kprintf("%d: Current entry length: %lld\n", i, curr_entry.length);
            kprintf("%d: Num frames: %lld\n", i, curr_entry.length / PAGE_SIZE);
            kprintf("%d: Reserved pages: %lld\n", i, reserved_pages);
            break;
        }
    }

    /* Now we need to re-set the frames that the bitmap occupies */
    uint64_t bitmap_phys = (uint64_t)bitmap - bl_get_hhdm_offset();

    kprintf("PMM_SET:   Setting %lld frames from 0x%llx to 0x%llx\n",
            ((bitmap_phys + bitmap_size) / PAGE_SIZE) - (bitmap_phys / PAGE_SIZE),
            bitmap_phys, bitmap_phys + bitmap_size);
    /* FIXME: Do I have <= here? */
    /* If I don't have it and bitmap is 40 frames and ends at 0x79000 and then call
    * pmm_alloc, it says it will alloc starting at 0x79000. But if I have <= it will
    * go xo 0x7a000 which makes more sense to me */
    /* NOTE: It is < and not <= because the upper address of a memory map entry
    * is EXCLUSIVE and not INCLUSIVE */
    for (uint64_t i = bitmap_phys / PAGE_SIZE; i < (bitmap_phys + bitmap_size) / PAGE_SIZE; i++) {
        pmm_set_frame(i);
    }

    unusable_pages = (max_usable_addr / PAGE_SIZE) - (reserved_pages + usable_pages);

    kprintf("Highest Usable Addr: 0x%llx\n", max_usable_addr);
    kprintf("Bitmap Size: 0x%llxB | %lld frames\n", bitmap_size, bitmap_size / PAGE_SIZE);
    kprintf("Bitmap Phys Start Addr: 0x%llx\n", (uint64_t)bitmap_phys);
    kprintf("Bitmap Phys End Addr: 0x%llx\n", (uint64_t)bitmap_phys + bitmap_size);
    kprintf("Bitmap Virt Start Addr: 0x%llx\n", (uint64_t)bitmap);
    kprintf("Bitmap Virt End Addr: 0x%llx\n", (uint64_t)bitmap + bitmap_size);
    kprintf("Total Page Frames: %lld\n", num_page_frames);
    kprintf("free pages: %lld\tused pages: %lld\nreserved pages: %lld\tusable pages: %lld\n", 
            free_pages,        used_pages,       reserved_pages,       usable_pages);
    kprintf("Total Page Frames from memmap: %lld\n", total_page_frames);
    kprintf("UNUSABLE Page Frames: %lld\n", unusable_pages);
    kprintf("Used (including reserved and bitmap) Page Frames: %lld\n", used_pages - unusable_pages);
    kprintf("ACTUALLY USED (from pmm_alloc) Page Frames: %lld\n", 
            used_pages - unusable_pages - reserved_pages - (bitmap_size / PAGE_SIZE));

    // kprintf("used_pages: %lld\n", used_pages);
    // kprintf("Allocing 5 frames\n");
    // void* test = pmm_alloc(5);
    // kprintf("used_pages after alloc: %lld\n", used_pages);
    // kprintf("Alloced 5 frames starting at 0x%llx\n", test);
    // kprintf("Freeing 5 frames\n");
    // pmm_free(test, 5);
    // kprintf("Allocing 12 frames\n");
    // void* test2 = pmm_alloc(12);
    // kprintf("used_pages after alloc: %lld\n", used_pages);
    // kprintf("Alloced 12 frames starting at 0x%llx\n", test2);
}

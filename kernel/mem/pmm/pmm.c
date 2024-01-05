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
            bitmap = (uint8_t*)(curr_entry.base + bl_get_hhdm_offset());
            break;
        }
    }

    /* Set all to 1 (non-free) */
    memset(bitmap, 0xFF, bitmap_size);
    used_pages = bitmap_size * WORD_LENGTH;

    /* Num entries - 1 because qemu has weird last reserved entry that is massive */
    for (uint64_t i = 0; i < memmap_get_num_entries() - 1; i++) {
        curr_entry = memmap_get_entry(i);

        switch (curr_entry.type) {
        case MEMMAP_USABLE:
            // kprintf("PMM_CLEAR: Clearing %lld frames from 0x%llx to 0x%llx\n",
            //         (curr_entry.length / PAGE_SIZE), curr_entry.base, curr_entry.base + curr_entry.length);
            /* FIXME: This ends up clearing 77 frames if (curr_entry.length / PAGE_SIZE) is equal to 78 (so 1 less) 
             * <= fixes that but idk if its the right thing to do*/
            for (uint64_t j = curr_entry.base / PAGE_SIZE; j < (curr_entry.base + curr_entry.length) / PAGE_SIZE; j++) {
                pmm_clear_frame(j);
            }
            usable_pages += curr_entry.length / PAGE_SIZE;
            break;
        case MEMMAP_KERNEL_AND_MODULES:
        case MEMMAP_RESERVED:              /* Fallthrough */
        case MEMMAP_ACPI_RECLAIMABLE:
        case MEMMAP_ACPI_NVS:
        case MEMMAP_BAD_MEMORY:
        case MEMMAP_BOOTLOADER_RECLAIMABLE:
        case MEMMAP_FRAMEBUFFER:
            reserved_pages += curr_entry.length / PAGE_SIZE;
            break;
        }
    }

    /* Now we need to re-set the frames that the bitmap occupies */
    uint64_t bitmap_phys = (uint64_t)bitmap - bl_get_hhdm_offset();

    kprintf("PMM_SET:   Setting %lld frames from 0x%llx to 0x%llx\n",
            ((bitmap_phys + bitmap_size) / PAGE_SIZE) - (bitmap_phys / PAGE_SIZE),
            bitmap_phys, bitmap_phys + bitmap_size);

    /* FIXME: Same issue as above with the clearing */
    for (uint64_t i = bitmap_phys / PAGE_SIZE; i <= (bitmap_phys + bitmap_size) / PAGE_SIZE; i++) {
        pmm_set_frame(i);
    }

    kprintf("Highest Usable Addr: 0x%llx | %lld\n", max_addr, max_addr);
    kprintf("Page Frames: %lld\n", num_page_frames);
    kprintf("Bitmap Size: 0x%llxB | %lld frames\n", bitmap_size, bitmap_size / PAGE_SIZE);
    kprintf("Bitmap Phys Start Addr: 0x%llx\n", (uint64_t)bitmap_phys);
    kprintf("Bitmap Phys End Addr: 0x%llx\n", (uint64_t)bitmap_phys + bitmap_size);
    kprintf("Bitmap Virt Start Addr: 0x%llx\n", (uint64_t)bitmap);
    kprintf("Bitmap Virt End Addr: 0x%llx\n", (uint64_t)bitmap + bitmap_size);

    kprintf("used_pages: %lld\n", used_pages);
    kprintf("Allocing 5 frames\n");
    void* test = pmm_alloc(5);
    kprintf("used_pages after alloc: %lld\n", used_pages);
    kprintf("Alloced 5 frames starting at 0x%llx\n", test);
    kprintf("Freeing 5 frames\n");
    pmm_free(test, 5);
    kprintf("Allocing 12 frames\n");
    void* test2 = pmm_alloc(12);
    kprintf("used_pages after alloc: %lld\n", used_pages);
    kprintf("Alloced 12 frames starting at 0x%llx\n", test2);
    kprintf("free pages: %lld\tused pages: %lld\treserved pages: %lld\tusable pages: %lld\n", 
            free_pages,        used_pages,       reserved_pages,       usable_pages);
}

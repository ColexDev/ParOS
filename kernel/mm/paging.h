#include <stdint.h>

#define PAGES_PER_TABLE 1024  /* 1 KiB */
#define TABLES_PER_DIR  1024  /* 1 KiB */
#define PAGE_SIZE       4096  /* 4 KiB */

#define PAGE_DIRECTORY_INDEX(virt) (((virt) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(virt) (((virt) >> 12) & 0x3ff)

/* This is just the frame number, aka the bitmap index because 
 * Right shifting 12 is the same as dividing by PAGE_SIZE */
#define GET_PAGE_FRAME(x) ((x) >> 12)

#define PAGE_PRESENT 1
#define PAGE_WRITABLE_BIT 1
#define PAGE_USER_BIT 2
#define PAGE_FRAME_SHIFT 12

#define CR0_PG_BIT 31
#define PAGE_TABLE_ADDRESS_MASK ~0xFFF

/* NOTES:
    * A Page is a fixed sized unit of memory     (VIRTUAL memory)
    * A Page is contained inside of a Page Frame (PHYSICAL memory)
*/

void init_paging();
uint32_t get_page(uint32_t virt, uint8_t make);
void alloc_page(uint32_t* page, uint8_t is_kernel, uint8_t is_writeable);

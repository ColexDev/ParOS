#include <stdint.h>

#define PAGES_PER_TABLE 1024  /* 1 KiB */
#define TABLES_PER_DIR  1024  /* 1 KiB */
#define PAGE_SIZE       4096  /* 4 KiB */

#define KERNEL_PHYS_BASE  0x100000
#define KERNEL_VIRT_BASE  0xC0000000
#define KERNEL_HEAP_START 0xF0000000
#define KERNEL_HEAP_END   0xFFBFFFFF
#define VMM_PAGE_TABLES   0xFFC00000
#define VMM_PAGE_DIR      0xFFFFF000

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

void init_paging();
uint32_t get_page(uint32_t virt);
void alloc_page(uint32_t* page, uint8_t is_kernel, uint8_t is_writeable);
void create_page_table(uint32_t virt);
void enable_paging(uint32_t* page_directory);
uint32_t* create_page_directory();
void map_kernel_into_page_directory(uint32_t* page_directory);
void set_page_directory(uint32_t* page_dir);
void use_global_page_directory(uint8_t enabled);

#include <stdint.h>

#define PAGES_PER_TABLE 0x400  // 1 KiB
#define TABLES_PER_DIR  0x400  // 1 KiB
#define PAGE_SIZE       0x1000 // 4 KiB
//
#define PAGE_DIRECTORY_INDEX(x) ((x) >> 22)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)

/* NOTES:
    * A Page is a fixed sized unit of memory     (VIRTUAL memory)
    * A Page is contained inside of a Page Frame (PHYSICAL memory)
*/

struct page 
{
    uint32_t present  : 1;
    uint32_t rw       : 1;
    uint32_t user     : 1;
    uint32_t accessed : 1;
    uint32_t dirty    : 1;
    uint32_t unused   : 8;
    uint32_t frame    : 20;
};

struct page_table
{
    struct page pages[PAGES_PER_TABLE];
};

struct page_directory
{
    struct page_table* tables[TABLES_PER_DIR];
};

void init_paging();

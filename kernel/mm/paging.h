#include <stdint.h>

#define PAGES_PER_TABLE 0x400  // 1 KiB
#define TABLES_PER_DIR  0x400  // 1 KiB
#define PAGE_SIZE       0x1000 // 4 KiB

#define PAGE_DIRECTORY_INDEX(x) ((x) >> 22)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)

/* This is just the frame number, aka the bitmap index because 
 * Right shifting 12 is the same as dividing by PAGE_SIZE */
#define GET_PAGE_FRAME(x) ((x) >> 12)

#define PRESENT 1

/* NOTES:
    * A Page is a fixed sized unit of memory     (VIRTUAL memory)
    * A Page is contained inside of a Page Frame (PHYSICAL memory)
*/

void init_paging();

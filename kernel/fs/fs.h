#include <stdint.h>

/* The first 10 MB (0x5000 lba's) of the disk will be used to store these nodes 
 * 2 MB for directories and 8 MB for files, this means that it can support
 * 16384 directories and 262144 files */

struct __attribute__((packed)) file_node {
    uint32_t size; /* In lba's */
    uint32_t start_lba;
    uint16_t id;
    char* name; /* max 22 characters long */
};

struct __attribute__((packed)) directory_node {
    uint16_t id;
    /* max 52 files per directory */
    uint16_t* files; /* stores all file ids in directory */
    char* name; /* max 22 characters long */
};

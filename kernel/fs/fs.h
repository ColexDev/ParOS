#include <stdint.h>

/* The first 1 MB (0x800 lba's) of the disk will be used to store these nodes 
 * 2 KB for directories and 8 KB for files, this means that it can support
 * 1638 directories and 26214 files */

struct __attribute__((packed)) file_node {
    uint8_t checksum;
    uint32_t size; /* In byte's */
    uint16_t id;
    uint8_t type; /* TYPE_FILE or TYPE_DIRECTORY */
    // char timestamp[14]; /* YYYYMMDDHHMMSS */
    uint32_t pointers[25]; /* each points to a sector, supports 12.8kb files */
    char name[20];
};

struct __attribute__((packed)) directory_node {
    uint16_t id;
    char timestamp[14]; /* YYYYMMDDHHMMSS */
    char name[22];
};

struct file_descriptor {
    uint32_t size;
    uint32_t offset;
    uint16_t id;
    uint8_t  flags;
};

#define FILE_OVERWRITE_FLAG 0
#define FILE_APPEND_FLAG    1

#define DATA_LBA_OFFSET  10
#define NODES_LBA_OFFSET 6

#define SECTOR_SIZE 512

#define MAX_FILE_NODES 64

#define NODE_CHECKSUM 251 /* 0xfb */

#define TYPE_FILE      0
#define TYPE_DIRECTORY 1

#define NODE_BITMAP_SIZE 8
#define DATA_BITMAP_SIZE 2560

void clear_sector(uint32_t lba);
void create_file(char* name);
void delete_file(char* name);
uint32_t open_file(char* name, uint8_t flags);
void write_fs_header();
void read_fs_header();
void list_files();
void write_file(uint32_t id, uint8_t* contents, uint32_t count);
void read_file(uint32_t id, uint8_t* buf, uint32_t count);
uint32_t get_file_size(uint32_t id);

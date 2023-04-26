#include <stdint.h>

/* The first 1 MB (0x800 lba's) of the disk will be used to store these nodes 
 * 2 KB for directories and 8 KB for files, this means that it can support
 * 1638 directories and 26214 files */

struct __attribute__((packed)) file_node {
    uint32_t size; /* In lba's */
    uint32_t start_lba;
    uint16_t id;
    uint8_t checksum;
    // uint16_t dir_id;
    // char timestamp[14]; /* YYYYMMDDHHMMSS */
    char name[21];
};

struct __attribute__((packed)) directory_node {
    uint16_t id;
    char timestamp[14]; /* YYYYMMDDHHMMSS */
    char name[22];
};

#define WORD_LENGTH    0x8
#define WORD_OFFSET(b) ((b) / WORD_LENGTH)
#define BIT_OFFSET(b)  ((b) % WORD_LENGTH)

#define DATA_LBA_OFFSET  10
#define NODES_LBA_OFFSET 6

void clear_sector(uint32_t lba);
void create_file(char* name);
struct file_node* open_file(char* name);
void write_fs_header();
void read_fs_header();
void list_files();

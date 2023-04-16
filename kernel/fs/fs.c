#include <stdint.h>

#include "../drivers/disk.h"
#include "../mm/kheap.h"
#include "../stdlib/util.h"
#include "fs.h"

#define BYTES_IN_SECTOR 512

#define BYTES_IN_DIR_HEADER  0x200000
#define BYTES_IN_FILE_HEADER 0x800000

#define BYTES_IN_BLOCK 0x1000

#define BYTES_IN_FILE_NODE 32

/*
 * We start with 10 MiB of storage supported.
 * The layout will be:
 * 512 bytes    20480 bytes  16384 bytes  10 MiB
 * node_bitmap->data_bitmap->nodes->      data*/

/* FIXME: NOT CURRENTLY WRITING THESE TO DISK */
uint8_t node_bitmap[64];
uint8_t data_bitmap[2560];

void
fs_set_frame(uint8_t* bitmap, uint32_t frame)
{
    bitmap[WORD_OFFSET(frame)] |= (1 << BIT_OFFSET(frame));
}

void
fs_clear_frame(uint8_t* bitmap, uint32_t frame)
{
    bitmap[WORD_OFFSET(frame)] &= ~(1 << BIT_OFFSET(frame));
}

uint8_t
fs_get_frame(uint8_t* bitmap, uint32_t frame)
{
    uint8_t ret = bitmap[WORD_OFFSET(frame)] & (1 << BIT_OFFSET(frame));
    return ret != 0;
}

void
clear_sector(uint32_t lba)
{
    uint8_t* buffer = kmalloc(sizeof(uint8_t) * BYTES_IN_SECTOR);

    for (int i = 0; i < BYTES_IN_SECTOR; i++) {
        buffer[i] = 0;
    }

    ata_write_sector(lba, buffer);

    kfree(buffer);
}

// uint32_t
// find_free_file_node()
// {
//     uint8_t* buffer = kmalloc(sizeof(uint8_t) * BYTES_IN_SECTOR);
//     uint32_t lba;
//     uint16_t sum = 0;
//
//     /* Loops 2 MiB - 10 MiB of disk */
//     for (int i = BYTES_IN_DIR_HEADER / BYTES_IN_SECTOR; i < (BYTES_IN_FILE_HEADER + BYTES_IN_DIR_HEADER) / BYTES_IN_SECTOR; i++) {
//         ata_read_sector(i, buffer);
//
//         for (int j = 0; j < BYTES_IN_SECTOR; j++) {
//             if (buffer[i] == 0) {
//                 sum++;
//             }
//         }
//
//         if (sum == BYTES_IN_SECTOR) {
//             lba = i;
//             break;
//         }
//     }
//
//     kfree(buffer);
//
//     return lba;
// }
//
// uint32_t
// find_free_directory_node()
// {
//     uint8_t* buffer = kmalloc(sizeof(uint8_t) * BYTES_IN_SECTOR);
//     uint32_t lba;
//     uint16_t sum = 0;
//
//     /* Loops through first 2 MiB of disk */
//     for (int i = 0; i < BYTES_IN_DIR_HEADER / BYTES_IN_SECTOR; i++) {
//         ata_read_sector(i, buffer);
//
//         for (int j = 0; j < BYTES_IN_SECTOR; j++) {
//             if (buffer[i] == 0) {
//                 sum++;
//             }
//         }
//
//         if (sum == BYTES_IN_SECTOR) {
//             lba = i;
//             break;
//         }
//     }
//
//     kfree(buffer);
//
//     return lba;
// }

uint32_t
find_space()
{
    for (uint32_t i = 0; i < 2560 * 8; i++) {
        uint8_t byte = data_bitmap[i];

        /* Move on if no 0 bits in byte */
        if (byte == 0xFF)
            continue;

        /* Get rightmost 0 bit (free frame) */
        uint8_t offset = __builtin_ctz(~byte);

        /* Finds the frame number */
        return i * WORD_LENGTH + offset;
    }

    return 0;
}

void
create_file(char* name)
{
    struct file_node f_node;
    uint8_t* buffer = kmalloc(sizeof(uint8_t) * BYTES_IN_SECTOR);
    uint8_t contents[5] = "Hello";

    memcpy(f_node.name, name, strlen(name));
    f_node.size = 0;
    f_node.id   = 0;
    f_node.start_lba = find_space() + DATA_LBA_OFFSET;

    fs_set_frame(node_bitmap, 0);

    for (int i = 0; i < 100; i++)
        fs_set_frame(data_bitmap, f_node.start_lba + i);
    
    ata_write_sector(0 + NODES_LBA_OFFSET, (uint8_t*)&f_node);

    /* Write hello to the file */
    ata_write_sector(f_node.start_lba, contents);
}

struct file_node*
open_file(char* name)
{
    uint8_t* buffer = kmalloc(512);
    struct file_node* fd;

    for (int i = NODES_LBA_OFFSET; i < 512 + NODES_LBA_OFFSET; i++) {
        kprintf("Reading sector...\n");
        ata_read_sector(i, buffer);
        /* 16 nodes per sector */
        for (int j = 0; j < BYTES_IN_SECTOR; i += 16) {
            fd = (struct file_node*)&buffer[j];
            if (kstrcmp(fd->name, name) == 0) {
                kprintf("NAME: %s\n", fd->name);
                kprintf("LBA: %d\n", fd->start_lba);
                return fd;
            }
        }
    }

    kfree(buffer);
}

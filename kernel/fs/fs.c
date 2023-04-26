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

#define NODE_CHECKSUM 251

/*
 * We start with 10 MiB of storage supported.
 * The layout will be:
 * 64 bytes     2560 bytes   2048 bytes  10 MiB
 * node_bitmap->data_bitmap->nodes->      data*/

/* NOTE: Node bitmap is overkill for only storing 64 nodes, could be 8 bytes */
uint8_t node_bitmap[64]    = {0};
uint8_t data_bitmap[2560]  = {0};
struct file_node nodes[64] = {0};

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
find_space(uint8_t* bitmap)
{
    for (uint32_t i = 0; i < 2560 * 8; i++) {
        uint8_t byte = bitmap[i];

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
    uint8_t contents[512] = "HELLO\n";

    memcpy(f_node.name, name, strlen(name));
    f_node.size = 0;
    f_node.id   = 0;
    f_node.start_lba = find_space(data_bitmap) + DATA_LBA_OFFSET;
    f_node.checksum = NODE_CHECKSUM;

    fs_set_frame(node_bitmap, find_space(node_bitmap));

    /* This is too complicated. 40 gives 50 lbas since we subtract 10 from i initally,
     * find a better way to do this */
    for (int i = f_node.start_lba - DATA_LBA_OFFSET; i < f_node.start_lba + 40; i++)
        fs_set_frame(data_bitmap, i);
    
    nodes[find_space(node_bitmap)] = f_node;

    /* Write hello to the file */
    ata_write_sector(f_node.start_lba, contents);
}

void
write_fs_header()
{
    uint8_t buffer[BYTES_IN_SECTOR] = {0};
    uint8_t j = 0;

    /* Write node bitmap (64 bytes/1 sector) */
    memcpy(buffer, node_bitmap, 64);
    ata_write_sector(0, buffer);

    /* Write data bitmap (2560 bytes/5 sectors) */
    for (uint8_t i = 1; i < NODES_LBA_OFFSET; i++) {
        memcpy(buffer, &data_bitmap[(i - 1) * BYTES_IN_SECTOR], BYTES_IN_SECTOR);
        ata_write_sector(i, buffer);
    }

    /* Write nodes (2048 bytes/4 sectors) */
    for (uint8_t i = NODES_LBA_OFFSET; i < DATA_LBA_OFFSET; i++) {
        memcpy(buffer, &nodes[j], BYTES_IN_SECTOR);
        ata_write_sector(i, buffer);
        j += 32;
    }
}

void
read_fs_header()
{
    uint8_t buffer[BYTES_IN_SECTOR];
    uint8_t j = 0;

    /* Read node bitmap (64 bytes/1 sector) */
    ata_read_sector(0, buffer);
    memcpy(node_bitmap, buffer, 64);

    /* Read data bitmap (2560 bytes/5 sectors) */
    for (uint8_t i = 1; i < NODES_LBA_OFFSET; i++) {
        ata_read_sector(i, buffer);
        memcpy(&data_bitmap[(i - 1) * BYTES_IN_SECTOR], buffer, BYTES_IN_SECTOR);
    }

    /* Read nodes (2048 bytes/4 sectors) */
    for (uint8_t i = NODES_LBA_OFFSET; i < DATA_LBA_OFFSET; i++) {
        ata_read_sector(i, buffer);
        // memcpy(&nodes[(i - NODES_LBA_OFFSET + 1) * BYTES_IN_SECTOR], buffer, BYTES_IN_SECTOR);
        memcpy(&nodes[j], buffer, BYTES_IN_SECTOR);
        j += 32;
    }
}

struct file_node*
open_file(char* name)
{
    uint8_t* buffer = kmalloc(512);
    uint8_t* contents = kmalloc(512);
    struct file_node fd;

    for (int j = 0; j < 64; j++) {
        fd = nodes[j];
        // memcpy(fd, &buffer[j], sizeof(struct file_node));

        if (kstrcmp(fd.name, name) == 0 && fd.checksum == NODE_CHECKSUM) {
            kprintf("NAME: %s\n", fd.name);
            kprintf("LBA: %d\n", fd.start_lba);
            ata_read_sector(fd.start_lba, contents);
            kprintf("CONTENTS: %s\n", contents);
            /* FIXME: PLEASE FIX THIS */
            return &fd;
        }
    }

    kfree(buffer);
    kfree(contents);
}

void
list_files()
{
    for (uint16_t i = 0; i < (sizeof(nodes) / sizeof(struct file_node)); i++) {
        /* Size if not currently used */
        if (nodes[i].checksum == NODE_CHECKSUM)
            kprintf("Name->%s\tID->%d\tLocation->%d\n", nodes[i].name, nodes[i].id, nodes[i].start_lba);
    }
}

#include <stdint.h>

#include "../drivers/disk.h"
#include "../mm/kheap.h"
#include "../stdlib/util.h"
#include "../stdlib/bitmap/bitmap.h"
#include "fs.h"

/*
 * We start with 10 MiB of storage supported.
 * The layout will be:
 * 64 bytes     2560 bytes   2048 bytes  10 MiB
 * node_bitmap->data_bitmap->nodes->      data*/

/* NOTE: Node bitmap is overkill for only storing 64 nodes, could be 8 bytes */
uint8_t node_bitmap[NODE_BITMAP_SIZE]  = {0};
uint8_t data_bitmap[DATA_BITMAP_SIZE]  = {0};
struct file_node nodes[MAX_FILE_NODES] = {0};

void
clear_sector(uint32_t lba)
{
    uint8_t* buffer = kmalloc(sizeof(uint8_t) * SECTOR_SIZE);

    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = 0;
    }

    ata_write_sector(lba, buffer);

    kfree(buffer);
}

void
create_file(char* name)
{
    struct file_node f_node;

    for (uint8_t i = 0; i < MAX_FILE_NODES; i++) {
        if (kstrcmp(nodes[i].name, name) == 0)
            return;
    }

    memcpy(f_node.name, name, strlen(name));
    f_node.size = 0;
    f_node.id   = find_first_free_bit(node_bitmap, NODE_BITMAP_SIZE);
    f_node.start_lba = find_first_free_bit(data_bitmap, DATA_BITMAP_SIZE) + DATA_LBA_OFFSET;
    f_node.checksum = NODE_CHECKSUM;

    set_bit(node_bitmap, find_first_free_bit(node_bitmap, NODE_BITMAP_SIZE));

    /* This is too complicated. 40 gives 50 lbas since we subtract 10 from i initally,
     * find a better way to do this */
    for (int i = f_node.start_lba - DATA_LBA_OFFSET; i < f_node.start_lba + 40; i++)
        set_bit(data_bitmap, i);
    
    nodes[find_first_free_bit(node_bitmap, NODE_BITMAP_SIZE) - 1] = f_node;
}

void
write_fs_header()
{
    uint8_t buffer[SECTOR_SIZE] = {0};
    uint8_t j = 0;

    /* Write node bitmap (64 bytes/1 sector) */
    memcpy(buffer, node_bitmap, 64);
    ata_write_sector(0, buffer);

    /* Write data bitmap (2560 bytes/5 sectors) */
    for (uint8_t i = 1; i < NODES_LBA_OFFSET; i++) {
        memcpy(buffer, &data_bitmap[(i - 1) * SECTOR_SIZE], SECTOR_SIZE);
        ata_write_sector(i, buffer);
    }

    /* Write nodes (2048 bytes/4 sectors) */
    for (uint8_t i = NODES_LBA_OFFSET; i < DATA_LBA_OFFSET; i++) {
        memcpy(buffer, &nodes[j], SECTOR_SIZE);
        ata_write_sector(i, buffer);
        j += 32;
    }
}

void
read_fs_header()
{
    uint8_t buffer[SECTOR_SIZE];
    uint8_t j = 0;

    /* Read node bitmap (64 bytes/1 sector) */
    ata_read_sector(0, buffer);
    memcpy(node_bitmap, buffer, 64);

    /* Read data bitmap (2560 bytes/5 sectors) */
    for (uint8_t i = 1; i < NODES_LBA_OFFSET; i++) {
        ata_read_sector(i, buffer);
        memcpy(&data_bitmap[(i - 1) * SECTOR_SIZE], buffer, SECTOR_SIZE);
    }

    /* Read nodes (2048 bytes/4 sectors) */
    for (uint8_t i = NODES_LBA_OFFSET; i < DATA_LBA_OFFSET; i++) {
        ata_read_sector(i, buffer);
        memcpy(&nodes[j], buffer, SECTOR_SIZE);
        j += 32;
    }
}

uint32_t
open_file(char* name)
{
    struct file_node fd;

    for (int j = 0; j < MAX_FILE_NODES; j++) {
        fd = nodes[j];

        if (kstrcmp(fd.name, name) == 0 && fd.checksum == NODE_CHECKSUM) {
            break;
        }
    }
    return fd.id;
}

void
write_file(uint32_t id, uint8_t* contents, uint32_t count)
{
    uint32_t num_sectors = count / SECTOR_SIZE;
    struct file_node fd = nodes[id];

    if (num_sectors == 0) num_sectors = 1;

    nodes[id].size = strlen((char*)contents);

    /* FIXME: STARTS WRITING FROM BEGINNING */
    for (int i = 0; i < num_sectors; i++) {
        ata_write_sector(fd.start_lba + i, &contents[i * SECTOR_SIZE]);
    }
}

/* change this to not always return the whole sector, only 
 * the number of bytes wanted */
void
read_file(uint32_t id, uint8_t* buf, uint32_t count)
{
    uint32_t num_sectors = count / SECTOR_SIZE;
    struct file_node fd = nodes[id];

    if (num_sectors == 0) num_sectors = 1;

    /* FIXME: STARTS WRITING FROM BEGINNING */
    for (int i = 0; i < num_sectors; i++) {
        ata_read_sector(fd.start_lba + i, &buf[i * SECTOR_SIZE]);
    }
}

uint32_t
get_file_size(uint32_t id)
{
    return nodes[id].size;
}

void
list_files()
{
    for (uint16_t i = 0; i < MAX_FILE_NODES; i++) {
        /* Size if not currently used */
        if (nodes[i].checksum == NODE_CHECKSUM)
            kprintf("Name->%s\tID->%d\tLocation->%d\tSize->%d\n", nodes[i].name, nodes[i].id, nodes[i].start_lba, nodes[i].size);
    }
}

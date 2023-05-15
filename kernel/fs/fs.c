#include <stdint.h>

#include "../drivers/disk.h"
#include "../mm/kheap.h"
#include "../stdlib/util.h"
#include "../stdlib/bitmap/bitmap.h"
#include "fs.h"

/*
 * We start with 10 MiB of storage supported.
 * The layout will be:
 * 8 bytes      2560 bytes   2048 bytes  10 MiB
 * node_bitmap->data_bitmap->nodes->      data*/

uint8_t node_bitmap[NODE_BITMAP_SIZE]  = {0};
uint8_t data_bitmap[DATA_BITMAP_SIZE]  = {0};
struct file_node nodes[MAX_FILE_NODES] = {0};

struct file_descriptor open_nodes[MAX_FILE_NODES] = {0};

void
clear_sector(uint32_t lba)
{
    uint8_t buffer[SECTOR_SIZE] = {0};

    ata_write_sector(lba, buffer);
}

void
create_file(char* name)
{
    struct file_node f_node = {0};

    for (uint8_t i = 0; i < MAX_FILE_NODES; i++) {
        if (kstrcmp(nodes[i].name, name) == 0)
            return;
    }

   memcpy(f_node.name, name, 20);
    f_node.checksum    = NODE_CHECKSUM;
    f_node.size        = 0;
    f_node.id          = find_first_free_bit(node_bitmap, NODE_BITMAP_SIZE);
    f_node.pointers[0] = find_first_free_bit(data_bitmap, DATA_BITMAP_SIZE) + DATA_LBA_OFFSET;
    kprintf("Initial pointer: %d\n", f_node.pointers[0]);

    set_bit(node_bitmap, f_node.id);

    /* Gives file one sector */
    set_bit(data_bitmap, f_node.pointers[0] - DATA_LBA_OFFSET);

    nodes[f_node.id] = f_node;
}

void
delete_file(char* name)
{
    uint32_t id = open_file(name, 0);
    if (id == 200)
        return;

    clear_bit(node_bitmap, id);

    /* FIXME: This may be very wrong, not tested yet */
    for (int i = 0; i < nodes[id].size / SECTOR_SIZE; i++) {
        clear_bit(data_bitmap, nodes[id].pointers[i] - DATA_LBA_OFFSET);
        clear_sector(nodes[id].pointers[i]);
    }
    // for (int i = nodes[id].start_lba - DATA_LBA_OFFSET; i < nodes[id].start_lba + 40; i++) {
    //     // clear_sector(i + DATA_LBA_OFFSET);
    //     clear_bit(data_bitmap, i);
    // }

    /* Clears the entry */
    memset(&nodes[id], 0, sizeof(struct file_node));
}

/* This is writing filenames twice?
 * also UNHANDLED is written after the filename */
void
write_fs_header()
{
    uint8_t buffer[SECTOR_SIZE] = {0};
    uint8_t j = 0;

    /* Write node bitmap (8 bytes/1 sector) */
    memcpy(buffer, node_bitmap, 8);
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
        j += (SECTOR_SIZE / sizeof(struct file_node));
    }
}

void
read_fs_header()
{
    uint8_t buffer[SECTOR_SIZE];
    uint8_t j = 0;

    /* Read node bitmap (8 bytes/1 sector) */
    ata_read_sector(0, buffer);
    memcpy(node_bitmap, buffer, 8);

    /* Read data bitmap (2560 bytes/5 sectors) */
    for (uint8_t i = 1; i < NODES_LBA_OFFSET; i++) {
        ata_read_sector(i, buffer);
        memcpy(&data_bitmap[(i - 1) * SECTOR_SIZE], buffer, SECTOR_SIZE);
    }

    /* Read nodes (2048 bytes/4 sectors) */
    for (uint8_t i = NODES_LBA_OFFSET; i < DATA_LBA_OFFSET; i++) {
        ata_read_sector(i, buffer);
        memcpy(&nodes[j], buffer, SECTOR_SIZE);
        j += (SECTOR_SIZE / sizeof(struct file_node));
    }
}

void
set_open_file_entry(uint32_t id, uint8_t flags)
{
    struct file_node fn = nodes[id];
    struct file_descriptor* fd = &open_nodes[id];

    fd->id = id;
    fd->size = fn.size;
    fd->flags = flags;
    fd->offset = 0;
}

uint32_t
open_file(char* name, uint8_t flags)
{
    struct file_node fn;

    for (int j = 0; j < MAX_FILE_NODES; j++) {
        fn = nodes[j];

        if (kstrcmp(fn.name, name) == 0 && fn.checksum == NODE_CHECKSUM) {
            set_open_file_entry(fn.id, flags);
            return fn.id;
        }
    }
    return 200;
}

uint32_t
get_next_pointer(uint32_t id)
{
    for (uint8_t i = 0; i < 25; i++)
        if (nodes[id].pointers[i] == 0)
            return i;
}

/* FIXME: If the last allocated sector is used, allocate another */
/* FIXME: Currently the pointers for other parts of the file
 * stay active even if we are only using the first one, if Overwrite
 * set all other pointers to 0 besides the first */
void
write_file(uint32_t id, uint8_t* contents, uint32_t count)
{
    uint32_t num_sectors      = count / SECTOR_SIZE;
    struct file_node* fn      = &nodes[id];
    uint32_t num_used_sectors = fn->size / SECTOR_SIZE;
    struct file_descriptor fd = open_nodes[id];

    if (num_sectors == 0) num_sectors = 1;

    /* Append */
    if (fd.flags & FILE_APPEND_FLAG) {
        uint8_t buf[SECTOR_SIZE * num_sectors];
        uint32_t file_size_sectors = get_file_size(id) / SECTOR_SIZE;
        uint32_t last_sector_size  = get_file_size(id) % SECTOR_SIZE;

        /* Sector will be used up, allocate a new one */
        kprintf("last_sector_size: %d\n", last_sector_size);
        kprintf("count: %d\n", count);
        if (last_sector_size + count > SECTOR_SIZE || (last_sector_size == 0 && get_file_size(id) != 0)) {
            uint32_t pointer      = get_next_pointer(id);
            kprintf("NEW POINTER: %d\n", pointer);
            fn->pointers[pointer] = find_first_free_bit(data_bitmap, DATA_BITMAP_SIZE) + DATA_LBA_OFFSET;;
            set_bit(data_bitmap, fn->pointers[pointer] - DATA_LBA_OFFSET);
            kprintf("NEW POINTER LOC: %d\n", fn->pointers[pointer]);
        }


        ata_read_sector(fn->pointers[0] + file_size_sectors, buf);
        memcpy(&buf[last_sector_size], contents, count);
        kprintf("num_sectors: %d\n", num_sectors);

        if (last_sector_size + count > SECTOR_SIZE) num_sectors++;

        uint8_t curr_pointer = get_file_size(id) / SECTOR_SIZE;
        kprintf("filesize: %d\n", get_file_size(id));
        kprintf("curr_pointer: %d\n", curr_pointer);

        for (int i = 0; i < num_sectors; i++) {
            ata_write_sector(fn->pointers[i + curr_pointer], &buf[i * SECTOR_SIZE]);
            kprintf("WRITING SECTOR %d at lba %d\n", i, fn->pointers[i + curr_pointer]);
        }
        fn->size += count;
    /* Overwrite */
    } else {
        for (int i = 0; i < num_sectors; i++) {
            ata_write_sector(fn->pointers[i], &contents[i * SECTOR_SIZE]);
        }
        fn->size = count;
    }
}

void
read_file(uint32_t id, uint8_t* buf, uint32_t count)
{
    uint32_t num_sectors = count / SECTOR_SIZE;
    struct file_node fn = nodes[id];
    char* temp_buf = kmalloc(sizeof(char) * count);

    if (count % SECTOR_SIZE != 0) num_sectors++;
    if (num_sectors == 0) num_sectors = 1;

    for (int i = 0; i < num_sectors; i++) {
        // ata_read_sector(fn.pointers[i], &temp_buf[i * SECTOR_SIZE]);
        ata_read_sector(fn.pointers[i], &buf[i * SECTOR_SIZE]);
        kprintf("READING SECTOR %d at lba %d\n", i, fn.pointers[i]);
    }
    
    /* I do this so only the data is returned and not 
     * necessarily an entire sector/part of sector not
     * used by this file */
    // memcpy(buf, temp_buf, count);
    // kfree(temp_buf);
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
        if (nodes[i].checksum == NODE_CHECKSUM)
            kprintf("Name->%s | ID->%d | Location->%d | Size->%d\n", nodes[i].name, nodes[i].id, nodes[i].pointers[0], nodes[i].size);
    }
}

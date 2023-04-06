#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define ATA_PRIMARY_DATA_PORT         0x1F0
#define ATA_PRIMARY_FEATURES_PORT     0x1F1
#define ATA_PRIMARY_SECTOR_COUNT_PORT 0x1F2
#define ATA_PRIMARY_LBA_LOW_PORT      0x1F3
#define ATA_PRIMARY_LBA_MID_PORT      0x1F4
#define ATA_PRIMARY_LBA_HIGH_PORT     0x1F5
#define ATA_PRIMARY_DRIVE_PORT        0x1F6
#define ATA_PRIMARY_COMMAND_PORT      0x1F7
#define ATA_PRIMARY_STATUS_PORT       0x1F7

#define ATA_STATUS_ERR    0x01
#define ATA_STATUS_DRQ    0x08
#define ATA_STATUS_READY  0x40
#define ATA_STATUS_BUSY   0x80

#define ATA_COMMAND_WRITE       0x30
#define ATA_COMMAND_READ        0x20
#define ATA_COMMAND_FLUSH_CACHE 0xE7

void ata_read_sector(uint32_t lba, uint8_t *buffer);
void ata_write_sector(uint32_t lba, uint8_t *buffer);

#endif /* #ifndef DISK_H */

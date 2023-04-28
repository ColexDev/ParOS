#include <stdint.h>

#include "../io/port_io.h"
#include "disk.h"
#include "../stdlib/util.h"

#define SECTOR_SIZE 512

void
ata_wait_bsy(void)
{
    while(inb(ATA_PRIMARY_STATUS_PORT) & ATA_STATUS_BUSY);
}

void
ata_wait_drdy(void)
{
    while(!(inb(ATA_PRIMARY_STATUS_PORT) & ATA_STATUS_READY));
}

void
ata_wait_drq(void)
{
    while(!(inb(ATA_PRIMARY_STATUS_PORT) & ATA_STATUS_DRQ));
}

void
ata_read_sector(uint32_t lba, uint8_t* buffer)
{
    ata_wait_bsy();

    outb(ATA_PRIMARY_SECTOR_COUNT_PORT, 1);
    outb(ATA_PRIMARY_LBA_LOW_PORT, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID_PORT, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HIGH_PORT, (lba >> 16) & 0xFF);
    outb(ATA_PRIMARY_DRIVE_PORT, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_COMMAND_PORT, ATA_COMMAND_READ);

    ata_wait_drdy();
    ata_wait_drq();

    for (int i = 0; i < SECTOR_SIZE; i += 2) {
        uint16_t word = inw(ATA_PRIMARY_DATA_PORT);
        buffer[i] = word & 0xFF;
        buffer[i + 1] = word >> 8;
    }

    ata_wait_bsy();
}

void
ata_write_sector(uint32_t lba, uint8_t* buffer) 
{
    ata_wait_bsy();

    outb(ATA_PRIMARY_SECTOR_COUNT_PORT, 1);
    outb(ATA_PRIMARY_LBA_LOW_PORT, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID_PORT, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HIGH_PORT, (lba >> 16) & 0xFF);
    outb(ATA_PRIMARY_DRIVE_PORT, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_COMMAND_PORT, ATA_COMMAND_WRITE);

    ata_wait_drdy();
    ata_wait_drq();

    for (int i = 0; i < SECTOR_SIZE; i += 2) {
        uint16_t word = (buffer[i + 1] << 8) | buffer[i];
        outw(0x1F0, word);
    }

    outb(ATA_PRIMARY_COMMAND_PORT, ATA_COMMAND_FLUSH_CACHE);

    ata_wait_bsy();
}

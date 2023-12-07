#include <stdint.h>

#include "sys.h"

void ata_400ns_delay(void)
{
	for (size_t i = 0; i < 15; ++i)
		inb(ATA_IO_BASE_PRIMARY + ATA_IO_STATUS);
}

void ata_poll(void)
{
    ata_400ns_delay();
    #define read status = inb(ATA_IO_BASE_PRIMARY + ATA_IO_STATUS)
    for (uint8_t read; ((status & (1 << ATA_IO_STATUS_BSY)) || !(status & (1 << ATA_IO_STATUS_DRQ)))
            && !(status & 1)
            && !(status & (1 << ATA_IO_STATUS_DF)); read);
}

// buffer - byte array to store count bytes read from the hard disk
// addr - starting byte address
// count - # of bytes to read
void ata_read(uint8_t* buffer, uint32_t addr, uint32_t count)
{
    uint32_t lba = addr / get_bytes_per_sector();
    uint32_t end_addr = addr + count;
    uint32_t sector_count = (1 + ((end_addr - 1) / get_bytes_per_sector())) - (addr / get_bytes_per_sector());
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_FEATURES, 0x00); // from wiki: Send a NULL byte to port 0x1F1, if you like (it is ignored and wastes lots of CPU time)
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_SECTOR_COUNT, sector_count); // sector count
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_SECTOR_NUM, (uint8_t) lba);               // L
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_CYLINDER_LO, (uint8_t) (lba >> 8));       // B
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_CYLINDER_HI, (uint8_t) (lba >> 16));      // A
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_CMD, 0x20); // READ SECTORS command byte
    for (uint32_t s = 0, bi = 0; s < sector_count; ++s)
    {
        ata_poll();
        for (size_t i = 0; i < get_bytes_per_sector() / 2; ++i)
        {
            uint16_t data = inw(ATA_IO_BASE_PRIMARY);
            size_t j = i * 2;
            if ((s || j >= (addr % get_bytes_per_sector())) && (s != sector_count - 1 || j <= ((end_addr - 1) % get_bytes_per_sector())))
                buffer[bi++] = data & 0xFF;
            ++j;
            if ((s || j >= (addr % get_bytes_per_sector())) && (s != sector_count - 1 || j <= ((end_addr - 1) % get_bytes_per_sector())))
                buffer[bi++] = (data >> 8) & 0xFF;
        }
        ata_400ns_delay();
    }
}

// buffer - byte array to write count bytes to the hard disk
// addr - starting byte address
// count - # of bytes to write
void ata_write(uint8_t* buffer, uint32_t addr, uint32_t count)
{
    uint32_t lba = addr / get_bytes_per_sector();
    uint32_t end_addr = addr + count;
    uint32_t sector_count = (1 + ((end_addr - 1) / get_bytes_per_sector())) - (addr / get_bytes_per_sector());

    uint32_t sector_local_addr = addr % get_bytes_per_sector();
    uint32_t sector_local_end_addr = end_addr % get_bytes_per_sector();

    uint32_t predata_length = sector_local_addr;
    uint8_t predata[max(predata_length, 1)];
    if (predata_length) ata_read(predata, lba * get_bytes_per_sector(), predata_length);
    uint32_t predata_ptr = 0;

    uint32_t postdata_length = get_bytes_per_sector() - sector_local_end_addr;
    uint8_t postdata[max(postdata_length, 1)];
    if (postdata_length) ata_read(postdata, end_addr, postdata_length);
    uint32_t postdata_ptr = 0;

    outb(ATA_IO_BASE_PRIMARY + ATA_IO_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_FEATURES, 0x00);
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_SECTOR_COUNT, sector_count); // sector count
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_SECTOR_NUM, (uint8_t) lba);               // L
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_CYLINDER_LO, (uint8_t) (lba >> 8));       // B
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_CYLINDER_HI, (uint8_t) (lba >> 16));      // A
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_CMD, 0x30); // WRITE SECTORS command byte
    for (uint32_t s = 0, bi = 0; s < sector_count; ++s)
    {
        ata_poll();
        for (size_t i = 0; i < get_bytes_per_sector() / 2; ++i)
        {
            size_t j = i * 2;
            bool write_lo = (s || j >= sector_local_addr) && (s != sector_count - 1 || j <= ((end_addr - 1) % get_bytes_per_sector()));
            ++j;
            bool write_hi = (s || j >= sector_local_addr) && (s != sector_count - 1 || j <= ((end_addr - 1) % get_bytes_per_sector()));
            uint16_t data = 0;
            if (write_lo)
                data |= buffer[bi++];
            else
                data |= ((!s && (i * 2) < sector_local_addr) ? predata[predata_ptr++] : postdata[postdata_ptr++]);
            if (write_hi)
                data |= ((uint16_t) (buffer[bi++]) << 8);
            else
                data |= ((uint16_t) ((!s && (i * 2 + 1) < sector_local_addr) ? predata[predata_ptr++] : postdata[postdata_ptr++]) << 8);
            outw(ATA_IO_BASE_PRIMARY, data);
        }
        ata_400ns_delay();
    }
}

/*
static void ata_write_sectors(uint8_t* buffer, uint32_t lba, uint8_t sector_count, uint16_t count)
{
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_FEATURES, 0x00);
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_SECTOR_COUNT, sector_count); // sector count
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_SECTOR_NUM, (uint8_t) lba);               // L
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_CYLINDER_LO, (uint8_t) (lba >> 8));       // B
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_CYLINDER_HI, (uint8_t) (lba >> 16));      // A
    outb(ATA_IO_BASE_PRIMARY + ATA_IO_CMD, 0x30); // WRITE SECTORS command byte
    for (size_t s = 0; s < sector_count; ++s)
    {
        ata_poll();
        for (size_t i = 0; i < count / 2; ++i)
            outw(ATA_IO_BASE_PRIMARY, *(uint16_t*)(buffer + (s * get_bytes_per_sector()) + (i * 2)));
        for (size_t i = count / 2; i < get_bytes_per_sector() / 2; ++i) // zero out the rest
            outw(ATA_IO_BASE_PRIMARY, 0x00);
        ata_400ns_delay();
    }
}
*/
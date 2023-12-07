#include "sys.h"

fat_BS_t fat_boot_record = {0};
fat_extBS_32_t* fat_boot_record_ext = NULL;
fat_FSInfo_t fat_fsinfo = {0};

//     uint32_t total_sectors = (fat_boot_record.total_sectors_16 == 0) ? fat_boot_record.total_sectors_32 : fat_boot_record.total_sectors_16;
//     uint32_t fat_size = (fat_boot_record.table_size_16 == 0) ? fat_boot_record_ext->table_size_32 : fat_boot_record.table_size_16;
//     uint32_t first_data_sector = fat_boot_record.reserved_sector_count + (fat_boot_record.table_count * fat_size);
//     uint32_t data_sectors = total_sectors - (fat_boot_record.reserved_sector_count + (fat_boot_record.table_count * fat_size));
//     uint32_t total_clusters = data_sectors / fat_boot_record.sectors_per_cluster;
//     uint32_t root_cluster = fat_boot_record_ext->root_cluster;

//     // print the OEM identifier to confirm the boot record was loaded properly
//     printf("FAT OEM identifier: %s\n", fat_boot_record.oem_name);
//     printf("bytes per sector: %i\n", fat_boot_record.bytes_per_sector);
//     printf("sectors per cluster: %i\n", fat_boot_record.sectors_per_cluster);
//     printf("FAT first sector: %i\n", fat_boot_record.reserved_sector_count);
//     printf("first data sector: %i\n", first_data_sector);
//     printf("# of sectors: %i\n", total_sectors);
//     printf("# of clusters: %i\n", total_clusters);
//     printf("# of data sectors: %i\n", data_sectors);
//     printf("root cluster: %i\n", root_cluster);

// buffer - a byte array of size sectors * bytes_per_sector
// cluster_no - number of the cluster to read
// sectors - # of sectors of the cluster to read, a number between 1 and sectors_per_cluster
void fat_read_cluster(uint8_t* buffer, uint32_t cluster_no, uint8_t sectors)
{
    if (sectors > fat_boot_record.sectors_per_cluster || !sectors) sectors = fat_boot_record.sectors_per_cluster;
    
    uint32_t fat_size = (fat_boot_record.table_size_16 == 0) ? fat_boot_record_ext->table_size_32 : fat_boot_record.table_size_16;
    uint32_t first_data_sector = fat_boot_record.reserved_sector_count + (fat_boot_record.table_count * fat_size);
    uint32_t first_sector_of_cluster = ((cluster_no - 2) * fat_boot_record.sectors_per_cluster) + first_data_sector;

    ata_reads(buffer, first_sector_of_cluster, sectors);
}

uint32_t fat_next_cluster(uint32_t cluster)
{
    uint16_t sector_size = fat_boot_record.bytes_per_sector;
    uint16_t first_fat_sector = fat_boot_record.reserved_sector_count;
    uint8_t FAT_table[sector_size];
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = first_fat_sector + (fat_offset / sector_size);
    uint32_t ent_offset = fat_offset % sector_size;

    ata_read((uint8_t*) FAT_table, fat_sector, ATA_READ_DEFAULT_COUNT);

    return (*(unsigned int*) &FAT_table[ent_offset]) & 0x0FFFFFFF;
}

uint32_t fat_strlen(char* str, uint32_t field_length)
{
    size_t count = 0;
    for (; str[field_length - 1 - count] == ' '; ++count);
    return field_length - count;
}

void fat_update_boot_record(void)
{
    ata_write((uint8_t*) &fat_boot_record, 0, sizeof fat_boot_record);
}

void fat_update_fsinfo(void)
{
    ata_write((uint8_t*) &fat_fsinfo, 1, sizeof fat_fsinfo);
}

uint8_t fat_checksum(uint8_t* full_filename)
{
    int16_t ffn_len;
    uint8_t sum;

    sum = 0;
    for (ffn_len = 11; ffn_len != 0; ffn_len--)
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *full_filename++;
    return sum;
}

void fat_read_directory_values(uint32_t directory_cluster, uint32_t* file_count, uint32_t* entry_count)
{
    uint8_t buffer[fat_boot_record.bytes_per_sector * fat_boot_record.sectors_per_cluster];

    uint32_t fc = 0, ec = 0;
    for (uint32_t cluster = directory_cluster; fat_is_good_cluster(cluster); cluster = fat_next_cluster(cluster))
    {
        fat_read_cluster(buffer, cluster, fat_boot_record.sectors_per_cluster);
        fat_file_t* files = (fat_file_t*) buffer;
        uint32_t files_len = (fat_boot_record.bytes_per_sector * fat_boot_record.sectors_per_cluster) / sizeof(fat_file_t);
        for (size_t i = 0; i < files_len; ++i, ++ec)
        {
            fat_file_t* cfile = files + i;
            uint8_t* raw = (uint8_t*) cfile;
            if (!raw[0])
                break;
            if (raw[0] == 0xE5)
                continue;
            if (raw[11] != 0x0F)
                ++fc;
        }
    }
    if (file_count) *file_count = fc;
    if (entry_count) *entry_count = ec;
}

void fat_init(void)
{
    // read the boot record, extension, and FSInfo into structs
    ata_read((uint8_t*) &fat_boot_record, 0, sizeof fat_boot_record);
    fat_boot_record_ext = (fat_extBS_32_t*) &(fat_boot_record.extended_section);
    ata_read((uint8_t*) &fat_fsinfo, 1, sizeof fat_fsinfo);
}
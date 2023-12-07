/* sys.h */

typedef struct fat_extBS_32_t
{
	//extended fat32 stuff
	unsigned int table_size_32;
	unsigned short extended_flags;
	unsigned short fat_version;
	unsigned int root_cluster;
	unsigned short fat_info;
	unsigned short backup_BS_sector;
	unsigned char reserved_0[12];
	unsigned char drive_number;
	unsigned char reserved_1;
	unsigned char boot_signature;
	unsigned int volume_id;
	unsigned char volume_label[11];
	unsigned char fat_type_label[8];
} __attribute__((packed)) fat_extBS_32_t;
 
typedef struct fat_extBS_16_t
{
	//extended fat12 and fat16 stuff
	unsigned char bios_drive_num;
	unsigned char reserved1;
	unsigned char boot_signature;
	unsigned int volume_id;
	unsigned char volume_label[11];
	unsigned char fat_type_label[8];
} __attribute__((packed)) fat_extBS_16_t;
 
typedef struct fat_BS_t
{
	unsigned char bootjmp[3];
	unsigned char oem_name[8];
	unsigned short bytes_per_sector;
	unsigned char sectors_per_cluster;
	unsigned short reserved_sector_count;
	unsigned char table_count;
	unsigned short root_entry_count;
	unsigned short total_sectors_16;
	unsigned char media_type;
	unsigned short table_size_16;
	unsigned short sectors_per_track;
	unsigned short head_side_count;
	unsigned int hidden_sector_count;
	unsigned int total_sectors_32;
	unsigned char extended_section[54];
} __attribute__((packed)) fat_BS_t;

typedef struct fat_FSInfo_t
{
	uint32_t lead_signature;
	uint8_t reserved[480];
	uint32_t mid_signature;
	uint32_t free_clusters;
	uint32_t last_cluster;
	uint8_t reserved_cont[12];
	uint32_t trail_signature;
} __attribute__((packed)) fat_FSInfo_t;

typedef struct fat_long_filename_t
{
	unsigned char order;
	unsigned short chars_first[5];
	unsigned char attribute;
	unsigned char long_entry_type;
	unsigned char checksum;
	unsigned short chars_mid[6];
	unsigned short zero;
	unsigned short chars_last[2];
} __attribute__((packed)) fat_long_filename_t;

typedef struct fat_file_t
{
	unsigned char filename[8];
	unsigned char fileext[3];
	unsigned char attrs;
	unsigned char reserved;
	unsigned char creation_time;
	unsigned short time_created;
	unsigned short date_created;
	unsigned short date_last_accessed;
	unsigned short cluster_no_hi;
	unsigned short time_last_modified;
	unsigned short date_last_modified;
	unsigned short cluster_no_lo;
	unsigned int filesize;
} __attribute__((packed)) fat_file_t;

extern fat_BS_t fat_boot_record;
extern fat_extBS_32_t* fat_boot_record_ext;
extern fat_FSInfo_t fat_fsinfo;

#define fat_is_good_cluster(cluster) (cluster < 0x0FFFFFF7)
#define fat_is_bad_cluster(cluster) (cluster == 0x0FFFFFF7)

void fat_init(void);
void fat_read_cluster(uint8_t* buffer, uint32_t cluster_no, uint8_t sectors);
uint32_t fat_next_cluster(uint32_t cluster);
uint32_t fat_strlen(char* str, uint32_t field_length);
void fat_update_boot_record(void);
void fat_update_fsinfo(void);
uint8_t fat_checksum(uint8_t* full_filename);
void fat_read_directory_values(uint32_t directory_cluster, uint32_t* file_count, uint32_t* entry_count);

/* io.c */

uint32_t abs_path_to_cluster(char* path)
{
    if (!strcmp(path, "/"))
        return fat_boot_record_ext->root_cluster;
    return 0;
}

bool fexists(fat_file_t* file, char* filename)
{
    uint8_t buffer[fat_boot_record.bytes_per_sector * fat_boot_record.sectors_per_cluster];
    char tmp_fn[256];
    tmp_fn[255] = '\0';
    char* tmp_fn_write = tmp_fn + 242;

    for (uint32_t cluster = fat_boot_record_ext->root_cluster; fat_is_good_cluster(cluster); cluster = fat_next_cluster(cluster))
    {
        fat_read_cluster(buffer, cluster, fat_boot_record.sectors_per_cluster);
        fat_file_t* files = (fat_file_t*) buffer;
        uint32_t files_len = (fat_boot_record.bytes_per_sector * fat_boot_record.sectors_per_cluster) / sizeof(fat_file_t);
        for (size_t i = 0; i < files_len; ++i)
        {
            fat_file_t* cfile = files + i;
            uint8_t* raw = (uint8_t*) cfile;
            if (!raw[0])
                break;
            if (raw[0] == 0xE5)
                continue;
            if (raw[11] == 0x0F)
            {
                if (tmp_fn_write < tmp_fn)
                    continue;
                fat_long_filename_t* long_fn = (fat_long_filename_t*) cfile;
                for (size_t j = 0; j < 5; ++j)
                    *tmp_fn_write++ = (uint8_t) long_fn->chars_first[j];
                for (size_t j = 0; j < 6; ++j)
                    *tmp_fn_write++ = (uint8_t) long_fn->chars_mid[j];
                for (size_t j = 0; j < 2; ++j)
                    *tmp_fn_write++ = (uint8_t) long_fn->chars_last[j];
                tmp_fn_write -= 26;
                continue;
            }

            tmp_fn_write += 13;
            if (tmp_fn_write == tmp_fn + 255) // no long filename
            {
                if (strcmp((char*) cfile->fileext, "   ")) // non-empty file extension
                {
                    uint32_t ext_len = fat_strlen((char*) cfile->fileext, sizeof cfile->fileext);
                    memcpy(tmp_fn_write -= ext_len, cfile->fileext, ext_len); // write extension to string
                    *(--tmp_fn_write) = '.'; // dot separator for extension
                }
                uint32_t name_len = fat_strlen((char*) cfile->filename, sizeof cfile->filename);
                memcpy(tmp_fn_write -= name_len, cfile->filename, name_len);
            }

            if (!strcmp(filename, tmp_fn_write))
            {
                memcpy(file, cfile, sizeof(fat_file_t));
                return true;
            }

            tmp_fn[255] = '\0';
            tmp_fn_write = tmp_fn + 242;
        }
    }
    return false;
}

bool fopen(fat_file_t* file, char* filename, char mode)
{
    bool e = fexists(file, filename);
    if (!e && (mode == FOPEN_READ || mode == FOPEN_READ_EXT))
        return false;
    return true;
}

// file - file to read from
// buffer - array to read the bytes into, must be of size count
// count - amount of bytes to read, must be a multiple of bytes_per_sector
void fread(fat_file_t* file, uint8_t* buffer, uint32_t count)
{
    uint32_t sectors_to_be_read = 1 + ((count - 1) / fat_boot_record.bytes_per_sector);
    uint32_t clusters_to_be_read = 1 + ((sectors_to_be_read - 1) / fat_boot_record.sectors_per_cluster); 
    uint32_t first_data_cluster = (((uint32_t) file->cluster_no_hi) << 16) | (uint32_t) file->cluster_no_lo;
    for (uint32_t i = 0, cluster = first_data_cluster; i < clusters_to_be_read; ++i, cluster = fat_next_cluster(cluster))
    {
        uint32_t sectors_read_iter = min(sectors_to_be_read, fat_boot_record.sectors_per_cluster);
        fat_read_cluster(buffer + (i * fat_boot_record.bytes_per_sector * fat_boot_record.sectors_per_cluster), cluster, sectors_read_iter);
        sectors_to_be_read -= sectors_read_iter;
    }
}

void fcreate(fat_file_t* file, char* filename)
{
    uint32_t full_fn_length = strlen(filename);
    uint32_t ext_i = 0;
    for (; ext_i < full_fn_length && filename[ext_i] != '.'; ++ext_i);
    if (ext_i >= full_fn_length) ext_i = 0xFFFFFFFF;
    uint32_t fn_length = ext_i == 0xFFFFFFFF ? full_fn_length : ext_i; 
    uint32_t ext_length = ext_i == 0xFFFFFFFF ? 0 : full_fn_length - ext_i - 1;

    // filename
    bool lfn_required = fn_length > 8 || ext_length > 3;
    if (lfn_required)
    {
        memcpy(file->filename, filename, 6);
        file->filename[6] = '~';
        file->filename[7] = '1';
    }
    else
    {
        memcpy(file->filename, filename, fn_length);
        memset(file->filename + fn_length, ' ', 8 - fn_length);
    }

    // extension
    memcpy(file->fileext, filename + fn_length + 1, min(ext_length, 3));
    if (ext_length < 3)
        memset(file->fileext + ext_length, ' ', 3 - ext_length);
    
    // attributes
    file->attrs = 0x00;

    // reserved
    file->reserved = 0x00;

    // creation time
    file->creation_time = 0x00;

    datetime_t dt;
    datetime(&dt);

    // time of file creation
    file->time_created = file->time_last_modified = (((uint16_t) (dt.hour & 0b11111)) << 11) | (((uint16_t) (dt.minute & 0b111111)) << 5) | (((uint16_t) (dt.second & 0b11111)) * 2);

    // date of file creation
    file->date_created = file->date_last_accessed = file->date_last_modified = (((uint16_t) (dt.year & 0b1111111)) << 9) |
        (((uint16_t) (dt.month & 0b1111)) << 5) | ((uint16_t) (dt.day & 0b11111));

    // first cluster number
    uint32_t first_cluster = fat_fsinfo.last_cluster += 1;

    fat_update_fsinfo(); // probably temporary because i don't have a proper method of shutting down the OS

    file->cluster_no_hi = first_cluster >> 16;
    file->cluster_no_lo = first_cluster | 0xFFFF;

    // size (0 bytes since it's a new file)
    file->filesize = 0;

    uint32_t directory_cluster = abs_path_to_cluster(active_directory);

    uint32_t file_count, entry_count;
    fat_read_directory_values(directory_cluster, &file_count, &entry_count);

    uint32_t lfns_length = lfn_required ? (1 + ((full_fn_length - 1) / 13)) : 0;

    uint8_t buffer[fat_boot_record.sectors_per_cluster * fat_boot_record.bytes_per_sector];

    if (lfn_required)
    {
        fat_long_filename_t lfns[lfns_length];
        memset(lfns, 0, lfns_length * sizeof(fat_long_filename_t));
        for (uint32_t i = lfns_length - 1; i >= 0; --i)
        {
            uint32_t ri = (uint32_t) ((int32_t) i - (int32_t) lfns_length + 1);
            fat_long_filename_t* lfn = lfns + i;
            lfn->order = ri + 1;
            if (!i)
                lfn->order |= 0x40;
            
            lfn->attribute = 0x0F;
            lfn->long_entry_type = 0x00;

            lfn->checksum = fat_checksum(file->filename);
            
            lfn->zero = 0x00;

            uint32_t fnci = 13 * ri;
            
            for (uint32_t j = 0; j < 5 && filename[fnci]; ++j, ++fnci)
                lfn->chars_first[j] = filename[fnci];
            
            for (uint32_t j = 0; j < 6 && filename[fnci]; ++j, ++fnci)
                lfn->chars_mid[j] = filename[fnci];
            
            for (uint32_t j = 0; j < 2 && filename[fnci]; ++j, ++fnci)
                lfn->chars_last[j] = filename[fnci];
        }
    }
}

/* terminal.c */

void terminal_exec()
{
	if (!lstrcmp(usertext, "ls", 2))
	{
		printf("in '%s':\n", active_directory);
		uint8_t buffer[fat_boot_record.bytes_per_sector * fat_boot_record.sectors_per_cluster];
		char tmp_fn[256];
		tmp_fn[255] = '\0';
		char* tmp_fn_write = tmp_fn + 242;

		for (uint32_t cluster = fat_boot_record_ext->root_cluster; fat_is_good_cluster(cluster); cluster = fat_next_cluster(cluster))
		{
			fat_read_cluster(buffer, cluster, fat_boot_record.sectors_per_cluster);
			fat_file_t* files = (fat_file_t*) buffer;
			uint32_t files_len = (fat_boot_record.bytes_per_sector * fat_boot_record.sectors_per_cluster) / sizeof(fat_file_t);
			for (size_t i = 0; i < files_len; ++i)
			{
				fat_file_t* cfile = files + i;
				uint8_t* raw = (uint8_t*) cfile;
				if (!raw[0])
					break;
				if (raw[0] == 0xE5)
					continue;
				if (raw[11] == 0x0F)
				{
					if (tmp_fn_write < tmp_fn)
						continue;
					fat_long_filename_t* long_fn = (fat_long_filename_t*) cfile;
					for (size_t j = 0; j < 5; ++j)
						*tmp_fn_write++ = (uint8_t) long_fn->chars_first[j];
					for (size_t j = 0; j < 6; ++j)
						*tmp_fn_write++ = (uint8_t) long_fn->chars_mid[j];
					for (size_t j = 0; j < 2; ++j)
						*tmp_fn_write++ = (uint8_t) long_fn->chars_last[j];
					tmp_fn_write -= 26;
					continue;
				}

				tmp_fn_write += 13;
				if (tmp_fn_write == tmp_fn + 255) // no long filename
				{
					if (strcmp((char*) cfile->fileext, "   ")) // non-empty file extension
					{
						uint32_t ext_len = fat_strlen((char*) cfile->fileext, sizeof cfile->fileext);
						memcpy(tmp_fn_write -= ext_len, cfile->fileext, ext_len); // write extension to string
						*(--tmp_fn_write) = '.'; // dot separator for extension
					}
					uint32_t name_len = fat_strlen((char*) cfile->filename, sizeof cfile->filename);
					memcpy(tmp_fn_write -= name_len, cfile->filename, name_len);
				}

				printf(" %s\n", tmp_fn_write);

				tmp_fn[255] = '\0';
				tmp_fn_write = tmp_fn + 242;
			}
		}
		return;
	}
	if (!lstrcmp(usertext, "cat", 3))
	{
		char* filename = usertext + 4;
		if (!*filename)
		{
			printf("provide a file to display\n");
			return;
		}
		fat_file_t file = {0};
		bool success = fopen(&file, filename, 'r');
		if (!success)
		{
			printf("file provided does not exist\n");
			return;
		}
		uint8_t buffer[FREAD_COUNT_ADJUST(file.filesize)];
		fread(&file, buffer, FREAD_COUNT_ADJUST(file.filesize));
		printf("%s\n", buffer);
		return;
	}
	if (!lstrcmp(usertext, "clear", 5))
	{
		terminal_clear();
		cursor_x = cursor_y = 0;
		update_cursor();
		return;
	}
	if (!lstrcmp(usertext, "help", 4))
	{
		printf("terminal help:\n");
		printf(" help          shows this screen\n");
		printf(" ls            displays the current directory\n");
		printf(" cat <file>    writes the contents of the file to the screen\n");
		printf(" clear         clears the screen\n");
		return;
	}
	printf("unknown command\n");
}
#include "sys.h"

superblock_base_t fs_superblock = {0};
superblock_ext_t* fs_superblock_ext = NULL;

inline uint32_t get_bytes_per_sector(void)
{
    return 512;
}

inline uint32_t get_bytes_per_inode(void)
{
    return fs_superblock_ext ? fs_superblock_ext->no_bytes_per_inode : 128;
}

static void fs_chk(void)
{
    if (fs_superblock.ext2_signature != 0xEF53)
    {
        printf("tried to access filesystem while disabled, halting\n");
        HALT_OS
    }
}

bool fs_absolute_path_to_inode(inode_t* dest, char* path)
{
    fs_chk();
    if (path[0] != '/' && path[0] != '\\') // not an absolute path
        return false;
    char tmp_directory[256] = {0};
    uint32_t tmp_dir_ptr = 0;
    inode_t tmp_inode = {0};
    uint32_t current_inode_no = 2;
    for (char c; (c = *path); ++path)
    {
        if (c == '/' || c == '\\')
        {
            fs_read_inode(&tmp_inode, current_inode_no);
            bool success = fs_find_inode(dest, &tmp_inode, tmp_dir_ptr ? tmp_directory : ".");
            current_inode_no = dest->number;
            if (!success)
                return false;
            memcpy(&tmp_inode, dest, sizeof(inode_t));
            memset(tmp_directory, 0, tmp_dir_ptr);
            tmp_dir_ptr = 0;
            continue;
        }
        tmp_directory[tmp_dir_ptr++] = c;
    }
    fs_read_inode(&tmp_inode, current_inode_no);
    bool success = fs_find_inode(dest, &tmp_inode, tmp_dir_ptr ? tmp_directory : ".");
    return success;
}

void fs_read_block_group_descriptor(block_group_descriptor_t* bgd, uint32_t block_group)
{
    fs_chk();
    uint32_t bytes_per_block = 1024 << fs_superblock.log_block_size;
    ata_read((uint8_t*) bgd, (bytes_per_block * (bytes_per_block == 1024 ? 2 : 1)) + (0x20 * block_group), sizeof(block_group_descriptor_t));
}

uint32_t fs_inode_diskaddr(uint32_t inode)
{
    fs_chk();
    uint32_t bytes_per_block = 1024 << fs_superblock.log_block_size;
    uint32_t block_group = (inode - 1) / fs_superblock.no_inodes_per_group;
    uint32_t index = (inode - 1) % fs_superblock.no_inodes_per_group;

    block_group_descriptor_t bgd = {0};
    fs_read_block_group_descriptor(&bgd, block_group);

    return (bgd.first_inode_table_block * bytes_per_block) + (index * get_bytes_per_inode());
}

void fs_read_inode(inode_t* dest, uint32_t inode)
{
    fs_chk();
    ata_read(((uint8_t*) dest) + sizeof(uint32_t), fs_inode_diskaddr(inode), sizeof(inode_t) - sizeof(uint32_t));
    dest->number = inode;
}

void fs_write_inode(inode_t* inode)
{
    fs_chk();
    ata_write(((uint8_t*) inode) + sizeof(uint32_t), fs_inode_diskaddr(inode->number), sizeof(inode_t) - sizeof(uint32_t));
}

// directory_inode: the directory to read from
// f: a function meant to handle each entry in the directory, this function will immediately return true if f returns true
//  - directory_entry_t* entry: the current directory entry
//  - char* entry_name: the name of the current directory entry
//  - block_group_descriptor_t* bgd: the block group descriptor for reading the entry's inode
//  - uint8_t* bargs: same as the bargs for this function
// bargs: the arguments for the outer function in a byte array
bool fs_for_each_entry(inode_t* directory_inode, bool (*f)(directory_entry_t*, char*, uint8_t*), uint8_t* bargs)
{
    fs_chk();
    uint32_t bytes_per_block = 1024 << fs_superblock.log_block_size;

    uint8_t buffer[directory_inode->size_lo];
    ata_read((uint8_t*) buffer, directory_inode->direct_block_ptr[0] * bytes_per_block, directory_inode->size_lo);

    for (uint32_t i = 0;;)
    {
        directory_entry_t* entry = (directory_entry_t*) (buffer + i);
        if (!entry->inode)
            break;
        i += sizeof(directory_entry_t);
        if (f(entry, (char*) (buffer + i), bargs))
            return true;
        i += entry->name_length_lo + (4 - (entry->name_length_lo % 4));
    }
    return false;
}

typedef struct fs_find_inode_args_t
{
    inode_t* dest;
    inode_t* directory_inode;
    char* filename;
} fs_find_inode_args_t;

bool fs_find_inode_for_each(directory_entry_t* entry, char* entry_name, uint8_t* bargs)
{
    fs_chk();
    fs_find_inode_args_t* args = (fs_find_inode_args_t*) bargs;
    if (!lstrcmp(args->filename, entry_name, strlen(args->filename)))
    {
        fs_read_inode(args->dest, entry->inode);
        return true;
    }
    return false;
}

bool fs_find_inode(inode_t* dest, inode_t* directory_inode, char* filename)
{
    unused(filename);
    fs_chk();
    fs_find_inode_args_t args = { .dest = dest, .directory_inode = directory_inode, .filename = filename };
    return fs_for_each_entry(directory_inode, fs_find_inode_for_each, (uint8_t*) &args);
}

void fs_read(inode_t* file, char* buffer)
{
    fs_chk();
    uint32_t bytes_per_block = 1024 << fs_superblock.log_block_size;
    if (file->triply_block_ptr || file->doubly_block_ptr || file->singly_block_ptr)
    {
        printf("unable to read files with indirect pointers yet, halting\n");
        HALT_OS
    }
    for (uint32_t i = 0, filesize = file->size_lo; i < 12; ++i, filesize -= bytes_per_block)
    {
        if (file->direct_block_ptr[i])
            ata_read((uint8_t*) (buffer + (i * bytes_per_block)), file->direct_block_ptr[i] * bytes_per_block, min(bytes_per_block, filesize));
    }
}

void fs_write(inode_t* file, char* buffer, uint32_t count)
{
    fs_chk();
    uint32_t new_size = count;
    uint32_t bytes_per_block = 1024 << fs_superblock.log_block_size;
    if (file->triply_block_ptr || file->doubly_block_ptr || file->singly_block_ptr)
    {
        printf("unable to write to files with indirect pointers yet, halting\n");
        HALT_OS
    }
    for (uint32_t i = 0; i < 12; ++i, count -= bytes_per_block)
    {
        if (file->direct_block_ptr[i])
            ata_write((uint8_t*) (buffer + (i * bytes_per_block)), file->direct_block_ptr[i] * bytes_per_block, min(bytes_per_block, count));
    }

    // update inode fields
    file->size_lo = new_size;
    fs_write_inode(file);
}

void fs_init(void)
{
    ata_read((uint8_t*) &fs_superblock, 1024, 1024);
    if (fs_superblock.major_version >= 1) fs_superblock_ext = (superblock_ext_t*) &(fs_superblock.extended);

    if (fs_superblock.ext2_signature == 0xEF53)
        printf("ext2 v%i.%i superblock loaded successfully\n", fs_superblock.major_version, fs_superblock.minor_version);
    else
        printf("failed to load ext2 superblock, filesystem access disabled\n");
}

// void read_root_directory(void)
// {
//     uint32_t bytes_per_block = 1024 << fs_superblock.log_block_size;
//     uint32_t bytes_per_fragment = 1024 << fs_superblock.log_fragment_size;
//     // printf("bytes per block: %i\n", bytes_per_block);
//     // printf("bytes per fragment: %i\n", bytes_per_fragment);
//     // printf("blocks per group: %i\n", fs_superblock.no_blocks_per_group);
//     // printf("inodes per group: %i\n", fs_superblock.no_inodes_per_group);
//     // printf("total # blocks: %i\n", fs_superblock.no_blocks);
//     // printf("total # inodes: %i\n", fs_superblock.no_inodes);
//     // printf("bytes per inode: %i\n", get_bytes_per_inode());

//     uint32_t inode = 2;
//     uint32_t block_group = (inode - 1) / fs_superblock.no_inodes_per_group;
//     uint32_t index = (inode - 1) % fs_superblock.no_inodes_per_group;
//     uint32_t containing_block = (index * get_bytes_per_inode()) / bytes_per_block;

//     block_group_descriptor_t bgd = {0};
//     ata_read((uint8_t*) &bgd, (bytes_per_block * (bytes_per_block == 1024 ? 2 : 1)) + (0x20 * block_group), sizeof(block_group_descriptor_t));
    
//     uint32_t root_dir_inode_addr = (bgd.first_inode_table_block * bytes_per_block) + (index * get_bytes_per_inode());
//     inode_t root_dir_inode = {0};
//     ata_read((uint8_t*) &root_dir_inode, root_dir_inode_addr, sizeof(inode_t));
    
//     uint8_t buffer[root_dir_inode.size_lo];
//     ata_read((uint8_t*) buffer, root_dir_inode.direct_block_ptr[0] * bytes_per_block, root_dir_inode.size_lo);

//     printf("files in root directory:\n");
//     for (uint32_t i = 0;;)
//     {
//         directory_entry_t* entry = (directory_entry_t*) (buffer + i);
//         if (!entry->inode)
//             break;
//         i += sizeof(directory_entry_t);
//         printf(" - %s\n", buffer + i);
//         i += entry->name_length_lo + (4 - (entry->name_length_lo % 4));
//     }
// }

// bool fs_find_inode(inode_t* dest, uint32_t* dest_no, uint32_t directory_inode, char* filename)
// {
//     uint32_t bytes_per_block = 1024 << fs_superblock.log_block_size;
//     uint32_t block_group = (directory_inode - 1) / fs_superblock.no_inodes_per_group;
//     uint32_t index = (directory_inode - 1) % fs_superblock.no_inodes_per_group;

//     block_group_descriptor_t bgd = {0};
//     ata_read((uint8_t*) &bgd, (bytes_per_block * (bytes_per_block == 1024 ? 2 : 1)) + (0x20 * block_group), sizeof(block_group_descriptor_t));

//     uint32_t dir_inode_addr = (bgd.first_inode_table_block * bytes_per_block) + (index * get_bytes_per_inode());
//     inode_t dir_inode = {0};
//     ata_read((uint8_t*) &dir_inode, dir_inode_addr, sizeof(inode_t));

//     uint8_t buffer[dir_inode.size_lo];
//     ata_read((uint8_t*) buffer, dir_inode.direct_block_ptr[0] * bytes_per_block, dir_inode.size_lo);

//     for (uint32_t i = 0;;)
//     {
//         directory_entry_t* entry = (directory_entry_t*) (buffer + i);
//         if (!entry->inode)
//             break;
//         i += sizeof(directory_entry_t);
//         if (!lstrcmp(filename, (char*) (buffer + i), strlen(filename)))
//         {
//             if (dest_no) *dest_no = entry->inode;
//             uint32_t file_index = (entry->inode - 1) % fs_superblock.no_inodes_per_group;
//             ata_read((uint8_t*) dest, (bgd.first_inode_table_block * bytes_per_block) + (file_index * get_bytes_per_inode()), sizeof(inode_t));
//             return true;
//         }
//         i += entry->name_length_lo + (4 - (entry->name_length_lo % 4));
//     }
//     return false;
// }
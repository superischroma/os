#ifndef SYS_H
#define SYS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define unused(x) (void)(x)
#define max(x, y) ((x) > (y) ? (x) : (y)) 
#define min(x, y) ((x) < (y) ? (x) : (y))

#define HALT_OS \
	asm volatile("cli; hlt"); \
    for (;;);

typedef struct
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;    
} registers_t;

typedef enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15
} vga_color;

/* fs.c */

typedef struct superblock_ext_t
{
	uint32_t first_nonreserved_inode;
	uint16_t no_bytes_per_inode;
	uint16_t superblock_block_group;
	uint32_t optional_features;
	uint32_t required_features;
	uint32_t ro_required_features;
	char fs_id[16];
	char volume_name[16];
	char volume_mount_path[64];
	uint32_t compression_used;
	uint8_t no_prealloc_blocks_files;
	uint8_t no_prealloc_blocks_directories;
	uint16_t unused;
	char journal_id[16];
	uint32_t journal_inode;
	uint32_t journal_device;
	uint32_t head_orphan_inodes;
} __attribute__((packed)) superblock_ext_t;

typedef struct superblock_base_t
{
	uint32_t no_inodes;
	uint32_t no_blocks;
	uint32_t no_blocks_reserved;
	uint32_t no_unalloc_blocks;
	uint32_t no_unalloc_inodes;
	uint32_t superblock_block_no;
	uint32_t log_block_size;
	uint32_t log_fragment_size;
	uint32_t no_blocks_per_group;
	uint32_t no_fragments_per_group;
	uint32_t no_inodes_per_group;
	uint32_t last_mount_time;
	uint32_t last_written_time;
	uint16_t no_mounts_since_check;
	uint16_t no_mounts_before_check;
	uint16_t ext2_signature;
	uint16_t fs_state;
	uint16_t error_action;
	uint16_t minor_version;
	uint32_t last_consistency_check;
	uint32_t consistency_check_interval;
	uint32_t creator_os_id;
	uint32_t major_version;
	uint16_t reserved_user_id;
	uint16_t reserved_group_id;
	uint8_t extended[sizeof(superblock_ext_t)];
} __attribute__((packed)) superblock_base_t;

#define FS_STATE_CLEAN 1
#define FS_STATE_ERROR 2

#define ERROR_ACTION_IGNORE 1
#define ERROR_ACTION_REMOUNT_RO 2
#define ERROR_ACTION_PANIC 3

#define OS_ID_LINUX 0
#define OS_ID_GNU_HURD 1
#define OS_ID_MASIX 2
#define OS_ID_FREEBSD 3
#define OS_ID_LITES 4

#define OPT_FEATURE_PREALLOC_BLOCKS_DIRECTORY 0x0001
#define OPT_FEATURE_AFS_SERVER_INODES 0x0002
#define OPT_FEATURE_FS_JOURNAL 0x0004
#define OPT_FEATURE_EXT_ATTR_INODES 0x0008
#define OPT_FEATURE_RESIZEABLE_FS 0x0010
#define OPT_FEATURE_HASH_DIRECTORIES 0x0020

#define REQ_FEATURE_COMPRESSION 0x0001
#define REQ_FEATURE_DIRECTORY_TYPE_FIELD 0x0002
#define REQ_FEATURE_REPLAY_JOURNAL 0x0004
#define REQ_FEATURE_JOURNAL_DEVICE 0x0008

#define RO_FEATURE_SPARSE 0x0001
#define RO_FEATURE_64BIT 0x0002
#define RO_FEATURE_DIRECTORY_BIN_TREE 0x0004

typedef struct block_group_descriptor_t
{
	uint32_t block_usage_block;
	uint32_t inode_usage_block;
	uint32_t first_inode_table_block;
	uint16_t no_unalloc_blocks;
	uint16_t no_unalloc_inodes;
	uint16_t no_directories;
} __attribute__((packed)) block_group_descriptor_t;

typedef struct inode_t
{
	uint32_t number;
	uint16_t type_permissions;
	uint16_t user_id;
	uint32_t size_lo;
	uint32_t last_accessed_time;
	uint32_t creation_time;
	uint32_t last_modified_time;
	uint32_t deletion_time;
	uint16_t group_id;
	uint16_t no_hard_links;
	uint32_t no_disk_sectors;
	uint32_t flags;
	uint32_t os_value1;
	uint32_t direct_block_ptr[12];
	uint32_t singly_block_ptr;
	uint32_t doubly_block_ptr;
	uint32_t triply_block_ptr;
	uint32_t no_generation;
	uint32_t ext_attr_block;
	uint32_t size_hi;
	uint32_t fragment_block_addr;
	uint8_t os_value2[12];
} __attribute__((packed)) inode_t;

#define INODE_TYPE_FIFO 0x1000
#define INODE_TYPE_CHAR_DEVICE 0x2000
#define INODE_TYPE_DIRECTORY 0x4000
#define INODE_TYPE_BLOCK_DEVICE 0x6000
#define INODE_TYPE_FILE 0x8000
#define INODE_TYPE_SYMB_LINK 0xA000
#define INODE_TYPE_SOCKET 0xC000

#define INODE_PERMS_OTHER_EXEC 0x001
#define INODE_PERMS_OTHER_WRITE 0x002
#define INODE_PERMS_OTHER_READ 0x004
#define INODE_PERMS_GROUP_EXEC 0x008
#define INODE_PERMS_GROUP_WRITE 0x010
#define INODE_PERMS_GROUP_READ 0x020
#define INODE_PERMS_USER_EXEC 0x040
#define INODE_PERMS_USER_WRITE 0x080
#define INODE_PERMS_USER_READ 0x100
#define INODE_PERMS_STICKY_BIT 0x200
#define INODE_PERMS_GROUP_ID 0x400
#define INODE_PERMS_USER_ID 0x800

#define INODE_FLAGS_SECURE_DELETION 0x00000001
#define INODE_FLAGS_KEEP_DELETED_DATA 0x00000002
#define INODE_FLAGS_FILE_COMPRESSION 0x00000004
#define INODE_FLAGS_SYNC_UPDATES 0x00000008
#define INODE_FLAGS_IMMUTABLE_FILE 0x00000010
#define INODE_FLAGS_APPEND_ONLY 0x00000020
#define INODE_FLAGS_NO_DUMP 0x00000040
#define INODE_FLAGS_NO_ACCESS_TIME_UPDATE 0x00000080
#define INODE_FLAGS_HASH_DIRECTORY 0x00010000
#define INODE_FLAGS_AFS_DIRECTORY 0x00020000
#define INODE_FLAGS_JOURNAL_FILE_DATA 0x00040000

typedef struct os_value2_linux_t
{
	uint8_t fragment_number;
	uint8_t fragment_size;
	uint16_t reserved1;
	uint16_t user_id_hi;
	uint16_t group_id_hi;
	uint32_t reserved2;
} __attribute__((packed)) os_value2_linux_t;

typedef struct directory_entry_t
{
	uint32_t inode;
	uint16_t size;
	uint8_t name_length_lo;
	uint8_t type_indicator;
} __attribute__((packed)) directory_entry_t;

uint32_t get_bytes_per_sector(void);
void fs_init(void);
void fs_read_inode(inode_t* dest, uint32_t inode);
bool fs_for_each_entry(inode_t* directory_inode, bool (*f)(directory_entry_t*, char*, uint8_t*), uint8_t* bargs);
bool fs_find_inode(inode_t* dest, inode_t* directory_inode, char* filename);
bool fs_absolute_path_to_inode(inode_t* dest, char* path);
void fs_read(inode_t* file, char* buffer);
void fs_write(inode_t* file, char* buffer, uint32_t count);

/* elf.c */

typedef struct elf_header_t
{
	uint32_t magic;
	uint8_t computing_bits;
	uint8_t endianness;
	uint8_t header_version;
	uint8_t os_abi;
	char unused[8];
	uint16_t content_type;
	uint16_t instruction_set;
	uint32_t elf_version;
	uint32_t program_entry_pos;
	uint32_t program_header_table_pos;
	uint32_t section_header_table_pos;
	uint32_t flags;
	uint16_t header_size;
	uint16_t program_header_table_entry_size;
	uint16_t program_header_table_size;
	uint16_t section_header_table_entry_size;
	uint16_t section_header_table_size;
	uint16_t section_names_index;
} __attribute__((packed)) elf_header_t;

#define ELF_HEADER_ISET_X86 0x03
#define ELF_HEADER_ISET_ARM 0x28
#define ELF_HEADER_ISET_X86_64 0x3E
#define ELF_HEADER_ISET_AARCH64 0xB7

typedef struct elf_program_header_t
{
	uint32_t segment_type;
	uint32_t data_offset;
	uint32_t virtual_addr_location;
	uint32_t unused;
	uint32_t file_segment_size;
	uint32_t memory_segment_size;
	uint32_t flags;
	uint32_t alignment;
} __attribute__((packed)) elf_program_header_t;

#define ELF_SEGMENT_TYPE_LOAD 1
#define ELF_SEGMENT_TYPE_DYNAMIC 2
#define ELF_SEGMENT_TYPE_INTERP 3
#define ELF_SEGMENT_TYPE_NOTE 4

#define ELF_PRGM_HEADER_FLAG_EXECUTABLE 0
#define ELF_PRGM_HEADER_FLAG_WRITABLE 1
#define ELF_PRGM_HEADER_FLAG_READABLE 2

bool elf_exec(uint8_t* binary);

/* gdt.c */
void gdt_init(void);

/* idt.c */
void idt_init(void);
void idt_add_descriptor(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);

/* mem.c */
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, void* src, size_t count);

/* isrs.c */
void isrs_init(void);

/* irq.c */
void irq_init(void);
void irq_add_handler(int irq, void (*handler)(registers_t* r));

/* keyboard.c */
void keyboard_init(void);

/* io.c */
#define FOPEN_EXTENDED 1

#define FOPEN_READ 'r'
#define FOPEN_WRITE 'w'
#define FOPEN_APPEND 'a'
#define FOPEN_READ_EXT ('r' + FOPEN_EXTENDED)
#define FOPEN_WRITE_EXT ('w' + FOPEN_EXTENDED)
#define FOPEN_APPEND_EXT ('a' + FOPEN_EXTENDED)

#define FREAD_COUNT_ADJUST(x) (fat_boot_record.bytes_per_sector * (1 + (((x) - 1) / fat_boot_record.bytes_per_sector)))

void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
int putc(int ch);
int printf(const char* fmt, ...);
int putchars(char* chars, uint32_t count);
int hex_dump(void* start, uint32_t count);

/* string.c */
int itoa(int value, char *sp, int radix, bool upper);
size_t strlen(char* str);
int strcmp(char* lhs, char* rhs);
int lstrcmp(char* lhs, char* rhs, uint32_t limit);

/* terminal.c */
extern char active_directory[];
extern unsigned cursor_x, cursor_y;
void terminal_init(void);
void terminal_put(char c, uint8_t color, size_t x, size_t y);
void enable_cursor(void);
void update_cursor(void);
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
int uputc(int ch);
void terminal_write_prompt(void);

/* rand.c */
long rand(void);
void srand(unsigned long seed);

/* time.c */
typedef struct datetime_t
{
	uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint32_t year;
} datetime_t;

void datetime(datetime_t* dest);
unsigned long time();

/* ata.c */

// ATA I/O base ports
#define ATA_IO_BASE_PRIMARY 0x1F0
#define ATA_IO_BASE_SECONDARY 0x170

// ATA I/O register offsets
#define ATA_IO_DATA 0x00
#define ATA_IO_ERROR 0x01
#define ATA_IO_FEATURES 0x01
#define ATA_IO_SECTOR_COUNT 0x02
#define ATA_IO_SECTOR_NUM 0x03
#define ATA_IO_CYLINDER_LO 0x04
#define ATA_IO_CYLINDER_HI 0x05
#define ATA_IO_DRIVE_HEAD 0x06
#define ATA_IO_STATUS 0x07
#define ATA_IO_CMD 0x07

// bits of the status register
#define ATA_IO_STATUS_ERR 0
#define ATA_IO_STATUS_IDX 1
#define ATA_IO_STATUS_CORR 2
#define ATA_IO_STATUS_DRQ 3
#define ATA_IO_STATUS_SRV 4
#define ATA_IO_STATUS_DF 5
#define ATA_IO_STATUS_RDY 6
#define ATA_IO_STATUS_BSY 7

#define ATA_READ_DEFAULT_COUNT ((uint16_t) 0x200)
#define ATA_WRITE_DEFAULT_COUNT ((uint16_t) 0x200)

void ata_read(uint8_t* buffer, uint32_t addr, uint32_t count);
void ata_write(uint8_t* buffer, uint32_t addr, uint32_t count);

#endif
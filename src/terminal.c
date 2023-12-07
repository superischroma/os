#include <stdint.h>

#include "sys.h"

char active_directory[256] = {0};
uint16_t active_directory_length = 0;
char usertext[256] = {0};
uint8_t usertext_ptr = 0;
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
uint16_t* terminal_buffer;

unsigned cursor_x, cursor_y;
 
uint8_t vga_entry_color(vga_color fg, vga_color bg) 
{
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_clear(void)
{
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
		}
	}
}
 
void terminal_init(void) 
{
	memcpy(active_directory, "/", active_directory_length = 1);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
		}
	}
}

void terminal_put(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

static bool ls_for_each_entry(directory_entry_t* entry, char* entry_name, uint8_t* bargs)
{
	unused(bargs);
	printf("  ");
	putchars(entry_name, entry->name_length_lo);
	printf("\n");
	return false;
}

void terminal_exec()
{
	if (!lstrcmp(usertext, "clear", 5))
	{
		terminal_clear();
		cursor_x = cursor_y = 0;
		update_cursor();
		return;
	}
	if (!lstrcmp(usertext, "help", 4))
	{
		printf("terminal command list:\n");
		printf("  help                       shows this list\n");
		printf("  ls [directory]             displays a directory\n");
		printf("  cat <file>                 writes the contents of a file to the screen\n");
		printf("  clear                      clears the screen\n");
		printf("  cd [directory]             changes the active directory\n");
		printf("  stat [file | directory]    displays information about a file or directory\n");
		return;
	}
	if (!lstrcmp(usertext, "ls", 2))
	{
		char* filepath = usertext + 3;
		uint32_t filepath_len = strlen(filepath);
		if (!filepath_len) filepath = active_directory;
		inode_t directory = {0};
		bool success = fs_absolute_path_to_inode(&directory, filepath);
		if (!success)
		{
			printf("could not read directory at path '%s'\n", filepath);
			return;
		}
		if ((directory.type_permissions & 0xF000) != 0x4000)
		{
			printf("the path specified is not a directory\n");
			return;
		}
		printf("in '%s':\n", filepath);
		fs_for_each_entry(&directory, ls_for_each_entry, NULL);
		return;
	}
	if (!lstrcmp(usertext, "cat", 3))
	{
		char* filepath = usertext + 4;
		uint32_t filepath_len = strlen(filepath);
		if (!filepath_len)
		{
			printf("provide a file to write to the screen\n");
			return;
		}
		inode_t file = {0};
		bool success = fs_absolute_path_to_inode(&file, filepath);
		if (!success)
		{
			printf("could not read file at path '%s'\n", filepath);
			return;
		}
		if ((file.type_permissions & 0xF000) != 0x8000)
		{
			printf("the path specified is not a file\n");
			return;
		}
		char buffer[file.size_lo];
		fs_read(&file, buffer);
		printf("%s\n", buffer);
		return;
	}
	if (!lstrcmp(usertext, "cd", 2))
	{
		char* filepath = usertext + 3;
		uint32_t filepath_len = strlen(filepath);
		if (!filepath_len)
		{
			filepath = "/";
			filepath_len = 1;
		}
		inode_t directory = {0};
		bool success = fs_absolute_path_to_inode(&directory, filepath);
		if (!success)
		{
			printf("could not enter directory at path '%s'\n", filepath);
			return;
		}
		if ((directory.type_permissions & 0xF000) != 0x4000)
		{
			printf("the path specified is not a directory\n");
			return;
		}
		memset(active_directory, 0, active_directory_length);
		memcpy(active_directory, filepath, active_directory_length = filepath_len);
		return;
	}
	if (!lstrcmp(usertext, "stat", 4))
	{
		char* filepath = usertext + 5;
		uint32_t filepath_len = strlen(filepath);
		if (!filepath_len)
		{
			printf("provide a file/directory to gather information about\n");
			return;
		}
		inode_t file = {0};
		bool success = fs_absolute_path_to_inode(&file, filepath);
		if (!success)
		{
			printf("could not read file/directory at path '%s'\n", filepath);
			return;
		}
		printf("  name: %s\n", filepath);
		printf("  type: ");
		switch (file.type_permissions & 0xF000)
		{
			case INODE_TYPE_FILE:
				printf("file\n");
				break;
			case INODE_TYPE_DIRECTORY:
				printf("directory\n");
				break;
			default:
				printf("unknown\n");
				break;
		}
		printf("  size: %i\n", file.size_lo);
		printf("  inode: %i\n", file.number);
		return;
	}
	if (!lstrcmp(usertext, "ee", 2))
	{
		char* filepath = usertext + 3;
		uint32_t filepath_len = strlen(filepath);
		if (!filepath_len)
		{
			printf("provide an executable to load\n");
			return;
		}
		inode_t file = {0};
		bool success_file = fs_absolute_path_to_inode(&file, filepath);
		if (!success_file)
		{
			printf("could not read file at path '%s'\n", filepath);
			return;
		}
		if ((file.type_permissions & 0xF000) != 0x8000)
		{
			printf("the path specified is not a file\n");
			return;
		}
		uint8_t buffer[file.size_lo];
		fs_read(&file, (char*) buffer);
		bool success_ee = elf_exec(buffer);
		if (!success_ee)
		{
			printf("failed to run executable\n");
			return;
		}
		return;
	}
	printf("unknown command\n");
}

void terminal_write_prompt(void)
{
	printf("'%s' > ", active_directory);
}

void enable_cursor(void)
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | 0x00);
 
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | 0xFF);
}

void update_cursor(void)
{
	uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

int uputc(int ch)
{
    putc(ch);
	if (ch == '\b')
		usertext[usertext_ptr ? --usertext_ptr : 0] = '\0';
	else if (ch == '\n')
	{
		terminal_exec();
		memset(usertext, 0, usertext_ptr);
		usertext_ptr = 0;
		terminal_write_prompt();
	}
	else
    	usertext[usertext_ptr++] = ch;
	return ch;
}
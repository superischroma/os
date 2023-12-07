/* credits for a lot of this work to both the OSdev wiki and Brandon F.'s kernel tutorial */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "sys.h"

const char* NAMES[] = {
	"Skibidi",
	"Money Bag",
	"Stupid",
	"Lit",
	"Jeffrey Dahmer",
	"Dumb",
	"\"Good\"",
	"Trash",
	"Fake",
	"Realest",
	"Kai Cenat Rizz Ohio",
	"Gyatt"
};

void kernel_main(void)
{
	gdt_init();
	idt_init();
	isrs_init();
	irq_init();
	keyboard_init();
	terminal_init();
	enable_cursor();
	cursor_x = cursor_y = 0;
	update_cursor();
	fs_init();
	srand(time());

	printf("Welcome to %s OS\n", NAMES[rand() % (sizeof(NAMES) / sizeof(NAMES[0]))]);
	terminal_write_prompt();

	for (;;);
}

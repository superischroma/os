/* credits to Brandon F. for most of this file: http://www.osdever.net/bkerndev/Docs/gettingstarted.htm */

#include "sys.h"

#define MOD_LSHIFT 0
#define MOD_RSHIFT 1
#define MOD_ALT 2
#define MOD_CTRL 3
#define MOD_CAPS 4

unsigned long modifiers = 0b00000000;

unsigned char translate[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
    '9', '0', '-', '=', '\b',	/* Backspace */
    '\t',			/* Tab */
    'q', 'w', 'e', 'r',	/* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',		/* Enter key */
    0,			/* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
    '\'', '`',   0,		/* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
    'm', ',', '.', '/',   0,					/* Right shift */
    '*',
    0,	/* Alt */
    ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
    '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
    '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

unsigned char shift_translate[128] =
{
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
    'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C',
    'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
    '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
    '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

void set_or_clear_bit(unsigned long* bits, int position, int state)
{
    if (state)
        (*bits) |= (1 << position);
    else
        (*bits) &= ~(1 << position);
}

int is_set_bit(unsigned long bits, int position)
{
    return (bits & (1 << position)) != 0;
}

void keyboard_handler(registers_t* r)
{
    unused(r);

    unsigned char scancode = inb(0x60);
    unsigned char key = is_set_bit(modifiers, MOD_LSHIFT) || is_set_bit(modifiers, MOD_RSHIFT) ? shift_translate[scancode] : translate[scancode];
    int releasing = scancode & 0x80;
    #define hold_entry(press, modifier) \
        case press: \
        case press + 0x80: \
            set_or_clear_bit(&modifiers, modifier, !releasing); \
            return;
    #define toggle_entry(press, modifier) \
        case press: \
            set_or_clear_bit(&modifiers, modifier, !is_set_bit(modifiers, modifier)); \
            return;
    switch (scancode)
    {
        hold_entry(0x2A, MOD_LSHIFT)
        hold_entry(0x36, MOD_RSHIFT)
        hold_entry(0x38, MOD_ALT)
        hold_entry(0x1D, MOD_CTRL)
        toggle_entry(0x3A, MOD_CAPS)
    }

    if (!releasing)
        uputc(key);
}

void keyboard_init(void)
{
    irq_add_handler(1, keyboard_handler);
}

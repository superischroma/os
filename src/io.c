#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "sys.h"

void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

void outw(uint16_t port, uint16_t val)
{
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

int radix_by_ident(char c)
{
    switch (c)
    {
        case 'x':
        case 'X':
            return 16;
        case 'o': return 8;
        case 'b': return 2;
        default: return 10;
    }
}

int putc(int ch)
{
    if (ch == '\b')
    {
        if (cursor_x)
            terminal_put(' ', vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK), --cursor_x, cursor_y);
        update_cursor();
        return ch;
    }
    if (ch == '\n')
    {
        cursor_x = 0;
        ++cursor_y;
        update_cursor();
        return ch;
    }
    if (ch == '\t')
    {
        for (uint8_t i = 0; i < 4; ++i)
            terminal_put(' ', vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK), cursor_x++, cursor_y);
        update_cursor();
        return ch;
    }
    terminal_put(ch, vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK), cursor_x++, cursor_y);
    update_cursor();
    return ch;
}

int printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    unsigned written = 0;
    for (char c; (c = *fmt); ++fmt, ++written)
    {
        if (c == '%')
        {
            if (!(c = *(++fmt)))
                continue;
            switch (c)
            {
                case '%':
                    putc('%');
                    ++written;
                    break;
                case 'i':
                case 'x':
                case 'X':
                case 'o':
                case 'b':
                    int i = va_arg(args, int);
                    char buffer[33];
                    itoa(i, buffer, radix_by_ident(c), c >= 'A');
                    for (unsigned j = 0; buffer[j]; ++j, ++written)
                        putc(buffer[j]);
                    break;
                case 's':
                    char* str = va_arg(args, char*);
                    written += printf(str);
                    break;
                case 'c':
                    i = va_arg(args, int);
                    putc(i);
                    ++written;
                    break;
            }
            continue;
        }
        putc(c);
    }
    va_end(args);
    return written;
}

int putchars(char* chars, uint32_t count)
{
    for (char c; (c = *chars) && count >= 1; ++chars, --count)
        putc(c);
    return count;
}

int hex_dump(void* start, uint32_t count)
{
    int written = 0;
    char text[0x11];
    text[0x10] = '\0';
    for (uint32_t i = 0; i < count; ++i)
    {
        uint8_t c = ((uint8_t*) start)[i];
        written += printf(c >= 0x10 ? "%X " : "0%X ", c);
        text[i % 0x10] = c >= ' ' && c <= '~' ? c : '.';
        if ((i & 0x0F) == 0x0F)
            written += printf("%s\n", text);
    }
    return written += printf("\n");
}
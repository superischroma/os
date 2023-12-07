/* credits to Brandon F. for most of this file: http://www.osdever.net/bkerndev/Docs/gettingstarted.htm */

#include <stdint.h>

#define GDT_ENTRIES 3

typedef struct
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) gdt_ptr_t;

gdt_entry_t gdt[GDT_ENTRIES];
gdt_ptr_t g_ptr;

extern void gdt_flush(gdt_ptr_t*);

void gdt_add_descriptor(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_init(void)
{
    g_ptr.limit = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    g_ptr.base = (uintptr_t) &gdt;

    gdt_add_descriptor(0, 0, 0, 0, 0);

    gdt_add_descriptor(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_add_descriptor(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    gdt_flush(&g_ptr);
}
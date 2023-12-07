/* credits to Brandon F. for most of this file: http://www.osdever.net/bkerndev/Docs/gettingstarted.htm */

#include <stdint.h>

#include "sys.h"

#define IDT_ENTRIES 256

typedef struct
{
    unsigned short base_lo;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_hi;
} __attribute__((packed)) idt_entry_t;

typedef struct
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) idt_ptr_t;

idt_entry_t idt[IDT_ENTRIES];
idt_ptr_t i_ptr;

extern void idt_load(idt_ptr_t* i_ptr);

void idt_add_descriptor(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags)
{
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = ((base >> 16) & 0xFFFF);

    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_init(void)
{
    i_ptr.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    i_ptr.base = (uintptr_t) &idt;

    memset(&idt, 0, sizeof(idt_entry_t) * IDT_ENTRIES);

    idt_load(&i_ptr);
}
/* credits to Brandon F. for most of this file: http://www.osdever.net/bkerndev/Docs/gettingstarted.htm */

#include "sys.h"

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

void* irq_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

void irq_add_handler(int irq, void (*handler)(registers_t* r))
{
    irq_routines[irq] = handler;
}

void irq_remove_handler(int irq)
{
    irq_routines[irq] = 0;
}

void irq_remap(void)
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void irq_init(void)
{
    irq_remap();

    idt_add_descriptor(32, (unsigned long) irq0, 0x08, 0x8E);
    idt_add_descriptor(33, (unsigned long) irq1, 0x08, 0x8E);
    idt_add_descriptor(34, (unsigned long) irq2, 0x08, 0x8E);
    idt_add_descriptor(35, (unsigned long) irq3, 0x08, 0x8E);
    idt_add_descriptor(36, (unsigned long) irq4, 0x08, 0x8E);
    idt_add_descriptor(37, (unsigned long) irq5, 0x08, 0x8E);
    idt_add_descriptor(38, (unsigned long) irq6, 0x08, 0x8E);
    idt_add_descriptor(39, (unsigned long) irq7, 0x08, 0x8E);

    idt_add_descriptor(40, (unsigned long) irq8, 0x08, 0x8E);
    idt_add_descriptor(41, (unsigned long) irq9, 0x08, 0x8E);
    idt_add_descriptor(42, (unsigned long) irq10, 0x08, 0x8E);
    idt_add_descriptor(43, (unsigned long) irq11, 0x08, 0x8E);
    idt_add_descriptor(44, (unsigned long) irq12, 0x08, 0x8E);
    idt_add_descriptor(45, (unsigned long) irq13, 0x08, 0x8E);
    idt_add_descriptor(46, (unsigned long) irq14, 0x08, 0x8E);
    idt_add_descriptor(47, (unsigned long) irq15, 0x08, 0x8E);
}

void irq_handler(registers_t* r)
{
    void (*handler)(registers_t* r) = irq_routines[r->int_no - 32];
    if (handler)
    {
        handler(r);
    }

    if (r->int_no >= 40)
    {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);
}

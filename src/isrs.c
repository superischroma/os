/* credits to Brandon F. for most of this file: http://www.osdever.net/bkerndev/Docs/gettingstarted.htm */

#include "sys.h"

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

void isrs_init(void)
{
    idt_add_descriptor(0, (unsigned long) isr0, 0x08, 0x8E);
    idt_add_descriptor(1, (unsigned long) isr1, 0x08, 0x8E);
    idt_add_descriptor(2, (unsigned long) isr2, 0x08, 0x8E);
    idt_add_descriptor(3, (unsigned long) isr3, 0x08, 0x8E);
    idt_add_descriptor(4, (unsigned long) isr4, 0x08, 0x8E);
    idt_add_descriptor(5, (unsigned long) isr5, 0x08, 0x8E);
    idt_add_descriptor(6, (unsigned long) isr6, 0x08, 0x8E);
    idt_add_descriptor(7, (unsigned long) isr7, 0x08, 0x8E);

    idt_add_descriptor(8, (unsigned long) isr8, 0x08, 0x8E);
    idt_add_descriptor(9, (unsigned long) isr9, 0x08, 0x8E);
    idt_add_descriptor(10, (unsigned long) isr10, 0x08, 0x8E);
    idt_add_descriptor(11, (unsigned long) isr11, 0x08, 0x8E);
    idt_add_descriptor(12, (unsigned long) isr12, 0x08, 0x8E);
    idt_add_descriptor(13, (unsigned long) isr13, 0x08, 0x8E);
    idt_add_descriptor(14, (unsigned long) isr14, 0x08, 0x8E);
    idt_add_descriptor(15, (unsigned long) isr15, 0x08, 0x8E);

    idt_add_descriptor(16, (unsigned long) isr16, 0x08, 0x8E);
    idt_add_descriptor(17, (unsigned long) isr17, 0x08, 0x8E);
    idt_add_descriptor(18, (unsigned long) isr18, 0x08, 0x8E);
    idt_add_descriptor(19, (unsigned long) isr19, 0x08, 0x8E);
    idt_add_descriptor(20, (unsigned long) isr20, 0x08, 0x8E);
    idt_add_descriptor(21, (unsigned long) isr21, 0x08, 0x8E);
    idt_add_descriptor(22, (unsigned long) isr22, 0x08, 0x8E);
    idt_add_descriptor(23, (unsigned long) isr23, 0x08, 0x8E);

    idt_add_descriptor(24, (unsigned long) isr24, 0x08, 0x8E);
    idt_add_descriptor(25, (unsigned long) isr25, 0x08, 0x8E);
    idt_add_descriptor(26, (unsigned long) isr26, 0x08, 0x8E);
    idt_add_descriptor(27, (unsigned long) isr27, 0x08, 0x8E);
    idt_add_descriptor(28, (unsigned long) isr28, 0x08, 0x8E);
    idt_add_descriptor(29, (unsigned long) isr29, 0x08, 0x8E);
    idt_add_descriptor(30, (unsigned long) isr30, 0x08, 0x8E);
    idt_add_descriptor(31, (unsigned long) isr31, 0x08, 0x8E);
}

const char* exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void fault_handler(registers_t* r)
{
    if (r->int_no < 32)
    {
        printf(exception_messages[r->int_no]);
        printf(" Exception. System Halted!\n");
        HALT_OS
    }
}
.global idt_load

idt_load:
    movl 4(%esp), %edx
    lidt (%edx)
    sti
    ret

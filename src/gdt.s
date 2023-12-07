.global gdt_flush

gdt_flush:
    mov 4(%esp), %edx
    lgdt (%edx)
    movl $0x10, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %fs
    movl %eax, %gs
    movl %eax, %ss
    jmp $0x08,$flush2
flush2:
    ret

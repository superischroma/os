    .global _start
s:
    .ascii "Hello\n"
_start:
    movl $4, %eax
    movl $1, %ebx
    movl $s, %ecx
    movl $6, %edx
    int $0x80

    movl $0, %eax
    movl $0, %ebx
    int $0x80

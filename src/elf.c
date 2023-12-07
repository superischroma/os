#include <stdint.h>
#include <stdbool.h>

#include "sys.h"

// todo
bool elf_exec(uint8_t* binary)
{
    elf_header_t* header = (elf_header_t*) binary;
    if (header->magic != 0x464C457F) // confirm binary passed in is an ELF executable
        return false;
    if (header->computing_bits != 1) // is binary not 32-bit?
        return false;
    elf_program_header_t* prgm_headers = (elf_program_header_t*) (binary + header->program_header_table_pos);
    return false;
}
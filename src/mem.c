#include <stddef.h>
#include <stdint.h>

void* memset(void* ptr, int value, size_t num)
{
    for (size_t i = 0; i < num; ++i)
        *(((uint8_t*) ptr) + i) = value;
    return ptr;
}

void* memcpy(void* dest, void* src, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        ((uint8_t*) dest)[i] = ((uint8_t*) src)[i];
    return dest;
}
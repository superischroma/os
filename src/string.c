#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

int itoa(int value, char *sp, int radix, bool upper)
{
    char tmp[33]; // be careful with the length of the buffer
    char *tp = tmp;
    int i;
    unsigned v;

    int sign = (radix == 10 && value < 0);    
    if (sign)
        v = -value;
    else
        v = (unsigned)value;

    while (v || tp == tmp)
    {
        i = v % radix;
        v /= radix;
        if (i < 10)
          *tp++ = i + '0';
        else
          *tp++ = i + (upper ? 'A' : 'a') - 10;
    }

    int len = tp - tmp;

    if (sign) 
    {
        *sp++ = '-';
        len++;
    }

    while (tp > tmp)
        *sp++ = *--tp;

    *sp = '\0';
    return len;
}

size_t strlen(char* str) 
{
	size_t i = 0;
	for (; str[i]; ++i);
	return i;
}

int strcmp(char* lhs, char* rhs)
{
    int cmp = 0;
    for (size_t i = 0; lhs[i] && rhs[i]; ++i)
        cmp += lhs[i] - rhs[i];
    return cmp;
}

int lstrcmp(char* lhs, char* rhs, uint32_t limit)
{
    int cmp = 0;
    for (size_t i = 0; lhs[i] && rhs[i] && i < limit; ++i)
        cmp += lhs[i] - rhs[i];
    return cmp;
}
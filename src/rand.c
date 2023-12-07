#include "sys.h"

typedef unsigned long time_t;

static long next = 1;

long rand(void)
{
    return (((next = next * 214013L + 2531011L) >> 16) & 0x7fff);
}

void srand(unsigned long seed)
{
    next = seed;
}
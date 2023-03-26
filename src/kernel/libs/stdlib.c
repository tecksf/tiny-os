#include "stdlib.h"
#include <x86.h>

#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

static unsigned long long next = 1;

int rand(void)
{
    next = (next * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1);
    unsigned long long result = (next >> 12);
    return (int) do_div(result, RAND_MAX + 1);
}

void seed_rand(unsigned int seed)
{
    next = seed;
}

uint32 hash32(uint32 val, unsigned int bits)
{
    uint32 hash = val * GOLDEN_RATIO_PRIME_32;
    return (hash >> (32 - bits));
}

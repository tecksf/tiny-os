#ifndef __LIBS_STDLIB_H__
#define __LIBS_STDLIB_H__

#include <defs.h>

#define RAND_MAX    2147483647UL

int rand(void);
void seed_rand(unsigned int seed);

uint32 hash32(uint32 val, unsigned int bits);

#endif // __LIBS_STDLIB_H__

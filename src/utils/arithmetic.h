#ifndef __UTILS_ARITHMETIC_H__
#define __UTILS_ARITHMETIC_H__

#include <defs.h>

struct Room
{
    uint32 gb;
    uint32 mb;
    uint32 kb;
    uint32 bytes;
};

struct Room calculate_room(uint64 bytes);

#endif //__UTILS_ARITHMETIC_H__

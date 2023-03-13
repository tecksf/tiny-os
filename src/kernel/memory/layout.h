#ifndef __MEMORY_LAYOUT_H__
#define __MEMORY_LAYOUT_H__

#include <defs.h>

#define E820MAX 20
#define E820_ARM 1 // AddressRangeMemory 这段内存可以被操作系统使用
#define E820_ARR 2 // AddressRangeReserved 内存使用中或者被系统保留，操作系统不可以使用此内存


// 实模式下通过BIOS 0x15中断，0xE820 子功能探测到的内存布局和大小
struct E820Map
{
    int nr_map;
    struct
    {
        uint64 address;
        uint64 size;
        uint32 type;
    } __attribute__((packed)) map[E820MAX];
};

#endif // __MEMORY_LAYOUT_H__

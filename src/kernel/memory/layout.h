#ifndef __MEMORY_LAYOUT_H__
#define __MEMORY_LAYOUT_H__

#include <defs.h>
#include <list.h>
#include <atomic.h>

#define E820_MAX 20
#define E820_ARM 1 // AddressRangeMemory 这段内存可以被操作系统使用
#define E820_ARR 2 // AddressRangeReserved 内存使用中或者被系统保留，操作系统不可以使用此内存

typedef uintptr pte;
typedef uintptr pde;

// 实模式下通过BIOS 0x15中断，0xE820 子功能探测到的内存布局和大小
struct E820Map
{
    int num;
    struct
    {
        uint64 address;
        uint64 size;
        uint32 type;
    } __attribute__((packed)) map[E820_MAX];
};

#define PAGE_RESERVED                   0
#define PAGE_PROPERTY                   1

// 描述一张4KB物理页
struct Page
{
    int ref;                        // 页的引用计数，被页表引用
    uint32 flags;                   // 表示该页是否被保留或者可用，如果是 PAGE_PROPERTY，则property字段有效
    unsigned int property;          // 用于首次适应算法，在可用区域内的第一个page里，记录其之后有多少个page是可用的
    ListEntry page_link;            // 用于将Page链接成一个双向链表
    ListEntry pra_page_link;        // 用来构造按页的最近访问时间进行排序的一个链表
    uintptr pra_vaddr;              // 记录物理页对应的虚拟页起始地址
};

#define SetPageReserved(page)       set_bit(PAGE_RESERVED, &((page)->flags))
#define ClearPageReserved(page)     clear_bit(PAGE_RESERVED, &((page)->flags))
#define IsPageReserved(page)        test_bit(PAGE_RESERVED, &((page)->flags))
#define SetPageProperty(page)       set_bit(PAGE_PROPERTY, &((page)->flags))
#define ClearPageProperty(page)     clear_bit(PAGE_PROPERTY, &((page)->flags))
#define IsPageProperty(page)        test_bit(PAGE_PROPERTY, &((page)->flags))

// 所有连续内存空闲快由一个双向链表管理，FreeArea.free_list则记录该链表的首尾空闲块的地址
// num 为空闲块的个数
struct FreeArea
{
    ListEntry free_list;
    unsigned int num;
};



#endif // __MEMORY_LAYOUT_H__

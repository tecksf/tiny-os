#ifndef __MEMORY_PAGE_H__
#define __MEMORY_PAGE_H__

#define NUM_PAGE_DIRECTORY_ENTRY        1024                    // 每个页目录表中的项个数
#define NUM_PAGE_TABLE_ENTRY            1024                    // 每个页表中的项个数

#define PAGE_SIZE       4096
#define PGSHIFT         12                      // log2(PAGE_SIZE)
#define PTSIZE          (PAGE_SIZE * NPTEENTRY)    // bytes mapped by a page directory entry
#define PTSHIFT         22                      // log2(PTSIZE)

#define PTXSHIFT        12                      // offset of PTX in a linear address
#define PDXSHIFT        22                      // offset of PDX in a linear address


// 线性地址结构可分为三部分
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |     Index      |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(la) --/ \--- PTX(la) --/ \---- PGOFF(la) ----/
//  \----------- PPN(la) -----------/

// 获取页目录索引
#define PDX(la) ((((uintptr)(la)) >> PDXSHIFT) & 0x3FF)

// 获取页表索引
#define PTX(la) ((((uintptr)(la)) >> PTXSHIFT) & 0x3FF)

// page number field of address
#define PPN(la) (((uintptr)(la)) >> PTXSHIFT)

// 页内偏移地址
#define PGOFF(la) (((uintptr)(la)) & 0xFFF)

// 构造线性地址
#define PGADDR(d, t, o) ((uintptr)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))
#define PTE_ADDR(pte)   ((uintptr)(pte) & ~0xFFF)
#define PDE_ADDR(pde)   PTE_ADDR(pde)

/* 页表项/页目录表项 flags */
#define PTE_P           0x001                   // 物理页是否存在
#define PTE_W           0x002                   // 物理页是否可写
#define PTE_U           0x004                   // 用户态是否可访问
#define PTE_PWT         0x008                   // 物理页是否为写直达，可以向更低级的储存设备写入
#define PTE_PCD         0x010                   // 物理页是否可以放入缓存
#define PTE_A           0x020                   // 表示物理页是否已经被访问
#define PTE_D           0x040                   // 脏位，表示对应的页是否写过数据
#define PTE_PS          0x080                   // 表示页的默认大小，0为4KB
#define PTE_MBZ         0x180                   // 该位必须为 0
#define PTE_AVAIL       0xE00                   // Available for software use
// The PTE_AVAIL bits aren't used by the kernel or interpreted by the
// hardware, so user processes are allowed to set them arbitrarily.

#define PTE_USER        (PTE_U | PTE_W | PTE_P)

#ifndef __ASSEMBLER__

#define E820_MAX 20
#define E820_ARM 1 // AddressRangeMemory 这段内存可以被操作系统使用
#define E820_ARR 2 // AddressRangeReserved 内存使用中或者被系统保留，操作系统不可以使用此内存

typedef uintptr pte;
typedef uintptr pde;

#include <list.h>
#include <atomic.h>

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

#endif // __ASSEMBLER__

#endif // __MEMORY_PAGE_H__

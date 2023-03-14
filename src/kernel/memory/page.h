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



#define KERNEL_BASE                     0xC0000000              // 内核线性地址基址 3G
#define KERNEL_MEMORY_SIZE              0x38000000              // 内核大小
#define KERNEL_TOP                      (KERNEL_BASE + KERNEL_MEMORY_SIZE)

#define VPT                             0xFAC00000

#define KERNEL_STACK_PAGE_COUNT         2
#define KERNEL_STACK_SIZE               (KERNEL_STACK_PAGE_COUNT * PAGE_SIZE)       // 内核栈大小，2页

#endif // __MEMORY_PAGE_H__

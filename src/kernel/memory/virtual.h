#ifndef __MEMORY_VIRTUAL_H__
#define __MEMORY_VIRTUAL_H__

#include "defs.h"
#include "list.h"

// 虚拟内存空间的三种属性
#define VM_READ                 0x00000001
#define VM_WRITE                0x00000002
#define VM_EXEC                 0x00000004
#define VM_STACK                0x00000008

// 包含所有虚拟内存空间的共同属性
struct VirtualMemory
{
    ListEntry mmap_list;                            // 指向由多个 VirtualMemoryArea 组成的链表
    struct VirtualMemoryArea *mmap_cache;           // 指向当前正在使用的 VirtualMemoryArea，根据局部性原理，通常不用再寻找链表
    pde *page_dir;                                  // 指向页目录表，所有 VirtualMemoryArea 的页目录表相同
    int map_count;                                  // VirtualMemoryArea 的数量
    void *swap_manager_private_data;                // the private data for swap manager
    int shared_count;                               // 使用当前内存空间的进程个数
};

struct VirtualMemoryArea
{
    struct VirtualMemory *virtual_memory;   // 所有vma的页目录表相同，因此都指向同一个 VirtualMemory
    uintptr start;                          // 虚拟内存起始地址
    uintptr end;                            // 虚拟内存结束地址
    uint32 flags;                           // 虚拟内存空间的属性
    ListEntry list_link;
};

#define OffsetOfVirtualMemoryArea(le, member) container_of((le), struct VirtualMemoryArea, member)

void virtual_memory_init(void);
int do_page_fault(struct VirtualMemory *memory, uint32 error_code, uintptr address);

struct VirtualMemory *create_virtual_memory(void);
void destroy_virtual_memory(struct VirtualMemory *memory);
int build_virtual_memory_mapping(struct VirtualMemory *memory, uintptr address, usize len, uint32 vm_flags);
int duplicate_virtual_memory_mapping(struct VirtualMemory *to, struct VirtualMemory *from);
void break_virtual_memory_mapping(struct VirtualMemory *memory);

struct VirtualMemoryArea *create_virtual_memory_area(uintptr start, uintptr end, uint32 flags);
struct VirtualMemoryArea *find_virtual_memory_area(struct VirtualMemory *memory, uintptr address);
void insert_virtual_memory_area(struct VirtualMemory *memory, struct VirtualMemoryArea *area);

bool user_memory_verification(struct VirtualMemory *memory, uintptr address, usize len, bool write);

static inline int get_shared_count(struct VirtualMemory *memory)
{
    return memory->shared_count;
}

static inline void set_shared_count(struct VirtualMemory *memory, int count)
{
    memory->shared_count = count;
}

static inline int increase_shared_count(struct VirtualMemory *memory)
{
    memory->shared_count += 1;
    return memory->shared_count;
}

static inline int decrease_shared_count(struct VirtualMemory *memory)
{
    memory->shared_count -= 1;
    return memory->shared_count;
}

#endif // __MEMORY_VIRTUAL_H__

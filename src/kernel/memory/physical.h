#ifndef __MEMORY_PHYSICAL_H__
#define __MEMORY_PHYSICAL_H__

#include <defs.h>
#include "layout.h"
#include "page.h"

extern usize num_of_physical_page;
extern struct Page *pages;

struct TaskState
{
    uint32 ts_link;       // 上一个任务的TSS指针
    uintptr ts_esp0;      // stack pointers and segment selectors
    uint16 ts_ss0;        // after an increase in privilege level
    uint16 ts_padding1;
    uintptr ts_esp1;
    uint16 ts_ss1;
    uint16 ts_padding2;
    uintptr ts_esp2;
    uint16 ts_ss2;
    uint16 ts_padding3;
    uintptr ts_cr3;       // page directory base
    uintptr ts_eip;       // saved state from last task switch
    uint32 ts_eflags;
    uint32 ts_eax;        // more saved state (registers)
    uint32 ts_ecx;
    uint32 ts_edx;
    uint32 ts_ebx;
    uintptr ts_esp;
    uintptr ts_ebp;
    uint32 ts_esi;
    uint32 ts_edi;
    uint16 ts_es;         // even more saved state (segment selectors)
    uint16 ts_padding4;
    uint16 ts_cs;
    uint16 ts_padding5;
    uint16 ts_ss;
    uint16 ts_padding6;
    uint16 ts_ds;
    uint16 ts_padding7;
    uint16 ts_fs;
    uint16 ts_padding8;
    uint16 ts_gs;
    uint16 ts_padding9;
    uint16 ts_ldt;
    uint16 ts_padding10;
    uint16 ts_t;          // trap on task switch
    uint16 ts_iomb;       // i/o map base address
} __attribute__((packed));

#define PageAddress(kva) ({                                                   \
            uintptr __m_kva = (uintptr)(kva);                       \
            if (__m_kva < KERNEL_BASE) {                                   \
            }                                                           \
            __m_kva - KERNEL_BASE;                                         \
        })

#define KADDR(pa) ({                                                    \
            uintptr __m_pa = (pa);                                    \
            usize __m_ppn = PPN(__m_pa);                               \
            if (__m_ppn >= npage) {                                     \
                                                                        \
            }                                                           \
            (void *) (__m_pa + KERNEL_BASE);                               \
        })


void physical_memory_init();

static inline struct Page* physical_address_to_page(uintptr pa)
{
    if (PageTableNum(pa) >= num_of_physical_page)
    {

    }
    return &pages[PageTableNum(pa)];
}


#endif // __MEMORY_PHYSICAL_H__

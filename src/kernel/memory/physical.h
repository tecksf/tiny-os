#ifndef __MEMORY_PHYSICAL_H__
#define __MEMORY_PHYSICAL_H__

#include <defs.h>
#include <assert.h>
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

#define PhysicalAddress(kva) ({                                                     \
            uintptr __m_kva = (uintptr)(kva);                                       \
            if (__m_kva < KERNEL_BASE) {                                            \
                panic("PhysicalAddress called with invalid kva %08lx", __m_kva);    \
            }                                                                       \
            __m_kva - KERNEL_BASE;                                                  \
        })

#define VirtualAddress(pa) ({                                                       \
            uintptr __m_pa = (pa);                                                  \
            usize __m_ppn = PageIndex(__m_pa);                                      \
            if (__m_ppn >= num_of_physical_page) {                                  \
                panic("VirtualAddress called with invalid pa %08lx", __m_pa);       \
            }                                                                       \
            (void *) (__m_pa + KERNEL_BASE);                                        \
        })


struct Page *allocate_pages(usize n);
void deallocate_pages(struct Page *base, usize n);
void *kernel_malloc(usize n);
void kernel_free(void *ptr, usize n);
void tlb_invalidate(pde *page_dir, uintptr linear_address);
void load_esp0(uintptr esp0);
pte *get_page_table_entry(pde *page_dir, uintptr linear_address, bool create);
int page_insert(pde *page_dir, struct Page *page, uintptr linear_address, uint32 perm);
void page_remove(pde *page_dir, uintptr linear_address);
struct Page *page_dir_alloc_page(pde *page_dir, uintptr linear_address, uint32 perm);

void physical_memory_init();
void print_page_table_item(uintptr address);

static inline ppn get_page_index(struct Page *page)
{
    return page - pages;
}

static inline uintptr page_to_physical_address(struct Page *page)
{
    return get_page_index(page) << PAGE_SHIFT;
}

static inline struct Page *physical_address_to_page(uintptr pa)
{
    if (PageIndex(pa) >= num_of_physical_page)
    {
        panic("physical_address_to_page called with invalid physical_address");
    }
    return &pages[PageIndex(pa)];
}

static inline void *page_to_virtual_address(struct Page *page)
{
    return VirtualAddress(page_to_physical_address(page));
}

static inline struct Page *virtual_address_to_page(void *kva)
{
    return physical_address_to_page(PhysicalAddress(kva));
}

static inline struct Page *page_table_entry_to_page(pte pte)
{
    if (!(pte & PTE_P))
    {
        panic("page_table_entry_to_page called with invalid pte");
    }
    return physical_address_to_page(PageTableEntryAddress(pte));
}

static inline struct Page *pde2page(pde pde)
{
    return physical_address_to_page(PageDirectoryEntryAddress(pde));
}


#endif // __MEMORY_PHYSICAL_H__

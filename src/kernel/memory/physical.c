#include "physical.h"
#include "gdt.h"
#include <env.h>
#include <error.h>
#include <x86.h>
#include <console.h>
#include <string.h>
#include <synchronous.h>
#include <arithmetic.h>
#include <swap.h>
#include "manager/first_fit.h"

struct Arch
{
    uint16 limit;       // 16位的表界限
    uintptr base;       // 32位的表基值
} __attribute__ ((packed));

static struct TaskState ts = {0};

static struct SegmentDescriptor gdt[] = {
        SegmentNull,
        [SEG_KTEXT] = Segment(STA_X | STA_R, 0x0, 0xFFFFFFFF, DPL_KERNEL),
        [SEG_KDATA] = Segment(STA_W, 0x0, 0xFFFFFFFF, DPL_KERNEL),
        [SEG_UTEXT] = Segment(STA_X | STA_R, 0x0, 0xFFFFFFFF, DPL_USER),
        [SEG_UDATA] = Segment(STA_W, 0x0, 0xFFFFFFFF, DPL_USER),
        [SEG_TSS] = SegmentNull,
};
static struct Arch gdtr_arch = {sizeof(gdt) - 1, (uintptr) gdt};

const struct PhysicalMemoryManager *memory_manager;
struct Page *pages;
usize num_of_physical_page = 0; // 物理内存的总页数

extern pde __boot_page_dir;
pde *boot_page_dir = &__boot_page_dir;
uintptr boot_cr3;

char *temp_user_space_start = NULL;

static inline void load_gdt(struct Arch *arch)
{
    asm volatile("lgdt (%0)"::"r"(arch));
    asm volatile("movw %%ax, %%gs"::"a"(USER_DS));
    asm volatile("movw %%ax, %%fs"::"a"(USER_DS));
    asm volatile("movw %%ax, %%es"::"a"(KERNEL_DS));
    asm volatile("movw %%ax, %%ds"::"a"(KERNEL_DS));
    asm volatile("movw %%ax, %%ss"::"a"(KERNEL_DS));
    // reload cs
    asm volatile("ljmp %0, $1f\n 1:\n"::"i"(KERNEL_CS));
}

void load_esp0(uintptr esp0)
{
    ts.ts_esp0 = esp0;
}

// 开启保护模式后，GDTR 中存储的地址就会按线性地址解析，原来在bootloader存储的
// 是物理地址。因此在保护模式下要再加载一次线性地址
void gdt_init()
{
    load_esp0((uintptr) boot_stack_top);
    ts.ts_ss0 = KERNEL_DS;

    // 在GDT中设置TSS段描述符
    gdt[SEG_TSS] = SegmentTSS(STS_T32A, (uintptr) &ts, sizeof(ts), DPL_KERNEL);

    // 在保护模式下，重新导入GDT表，此时GDTR中存的是线性地址
    load_gdt(&gdtr_arch);

    // 将TSS的段选择子导入到TR寄存器
    ltr(GD_TSS);
}

static void physical_memory_manager_init()
{
    memory_manager = &first_fit_memory_manager;
    kernel_print("memory management: %s\n", memory_manager->name);
    memory_manager->init();
}

static void memory_map_init(struct Page *base, usize n)
{
    memory_manager->init_memory_map(base, n);
}

// 探测空闲的物理内存，标记已使用的空间，再调用 memory_map_init 创建页链表
static void physical_page_init()
{
    // BIOS探测的内存信息存在物理地址0x8000
    struct E820Map *memory_map = (struct E820Map *) (0x8000 + KERNEL_BASE);
    uint64 max_physical_address = 0, start, finish;

    kernel_print("e820map: %d\n", memory_map->num);
    int i;
    for (i = 0; i < memory_map->num; i++)
    {
        start = memory_map->map[i].address;
        finish = start + memory_map->map[i].size;

        struct Room room = calculate_room(memory_map->map[i].size);

        kernel_print("  memory: size = %08llx(%2dG %3dM %3dK %3dB), [%08llx, %08llx], type = %d.\n",
                     memory_map->map[i].size, room.gb, room.mb, room.kb, room.bytes, start, finish - 1, memory_map->map[i].type);

        if (memory_map->map[i].type == E820_ADDRESS_RANGE_MEMORY)
        {
            if (max_physical_address < finish && start < KERNEL_MEMORY_SIZE)
            {
                max_physical_address = finish;
            }
        }
    }

    if (max_physical_address > KERNEL_MEMORY_SIZE)
    {
        max_physical_address = KERNEL_MEMORY_SIZE;
    }

    extern char end[];  // end 定义在 kernel.ld, bss段结束位置
    num_of_physical_page = max_physical_address / PAGE_SIZE;
    kernel_print("max physical address = %08llx, num of physical page = %d, end = %p\n", max_physical_address, num_of_physical_page, end);

    temp_user_space_start = (char *) RoundUp((void *) end, PAGE_SIZE);


    // 从end位置开始（按4096对齐后），分配Page结构体，第一个Page结构体永远对应第一个物理页，依次类推
    // 将内核所占的空间 KERNEL_MEMORY_SIZE， 对应的Page设置为已使用
    pages = (struct Page *) RoundUp((void *) (temp_user_space_start + PAGE_SIZE * 256), PAGE_SIZE);
    for (i = 0; i < num_of_physical_page; i++)
    {
        SetPageReserved(pages + i);
    }

    uintptr freemem = PhysicalAddress((uintptr) pages + sizeof(struct Page) * num_of_physical_page);
    kernel_print("freemem = %p\n", freemem);

    // 对空闲的内存（freemem之后）所对应的Page进行初始化
    for (i = 0; i < memory_map->num; i++)
    {
        start = memory_map->map[i].address;
        finish = start + memory_map->map[i].size;
        if (memory_map->map[i].type == E820_ADDRESS_RANGE_MEMORY)
        {
            if (start < freemem)
                start = freemem;

            if (finish > KERNEL_MEMORY_SIZE)
                finish = KERNEL_MEMORY_SIZE;

            if (start < finish)
            {
                kernel_print("[ start = %08llx, finish = %08llx ]\n", start, finish);
                start = RoundUp(start, PAGE_SIZE);
                finish = RoundDown((usize) finish, PAGE_SIZE);
                if (start < finish)
                {
                    memory_map_init(physical_address_to_page(start), (finish - start) / PAGE_SIZE);
                }
            }
        }
    }
}

struct Page *allocate_pages(usize n)
{
    struct Page *page = NULL;
    bool flag;
    SaveLocalInterrupt(flag);
    {
        page = memory_manager->alloc_pages(n);
    }
    RestoreLocalInterrupt(flag);

    return page;
}

void deallocate_pages(struct Page *base, usize n)
{
    bool flag;
    SaveLocalInterrupt(flag);
    {
        memory_manager->free_pages(base, n);
    }
    RestoreLocalInterrupt(flag);
}

void tlb_invalidate(pde *page_dir, uintptr linear_address)
{
    if (rcr3() == PhysicalAddress(page_dir))
    {
        invlpg((void *) linear_address);
    }
}

pte *get_page_table_entry(pde *page_dir, uintptr linear_address, bool create)
{
    pde *entry = &page_dir[PageDirectoryIndex(linear_address)];
    if (!(*entry & PTE_P))
    {
        struct Page *page;
        if (!create || (page = allocate_pages(1)) == NULL)
        {
            return NULL;
        }
        set_page_reference(page, 1);
        uintptr physical_address = page_to_physical_address(page);
        memory_set(VirtualAddress(physical_address), 0, PAGE_SIZE);
        *entry = physical_address | PTE_U | PTE_W | PTE_P;
    }
    uint32 index = PageTableIndex(linear_address);
    return &((pte *) VirtualAddress(PageDirectoryEntryAddress(*entry)))[index];
}

void physical_memory_init()
{
    boot_cr3 = PhysicalAddress(boot_page_dir);

    physical_memory_manager_init();

    physical_page_init();

    // 页目录表第1003项映射到页目录表自身
    boot_page_dir[PageDirectoryIndex(VPT)] = PhysicalAddress(boot_page_dir) | PTE_P | PTE_W;

    gdt_init();

//    print_page_table_item((uintptr) boot_page_dir);
}

void *kernel_malloc(usize n)
{
    void *ptr = NULL;
    struct Page *base = NULL;
    assert(n > 0 && n < 1024 * 0124);
    int num_pages = (n + PAGE_SIZE - 1) / PAGE_SIZE;
    base = allocate_pages(num_pages);
    assert(base != NULL);
    ptr = page_to_virtual_address(base);
    return ptr;
}

void kernel_free(void *ptr, usize n)
{
    assert(n > 0 && n < 1024 * 0124);
    assert(ptr != NULL);
    struct Page *base = NULL;
    int num_pages = (n + PAGE_SIZE - 1) / PAGE_SIZE;
    base = virtual_address_to_page(ptr);
    deallocate_pages(base, num_pages);
}

// 从页表中将页表项与页表的映射取消，即将页表项置为0
static inline void remove_page_and_page_table_entry(pde *page_dir, uintptr linear_address, pte *entry)
{
    // 判断页表项第0位是否为1,确认该页是否在物理内存中
    if (*entry & PTE_P)
    {
        struct Page *page = page_table_entry_to_page(*entry);
        if (decrease_page_reference(page) == 0)
        {
            deallocate_pages(page, 1);
        }
        *entry = 0;
        tlb_invalidate(page_dir, linear_address);
    }
}

// 设置页对应的页表项，即为页表新插入一张页
int page_insert(pde *page_dir, struct Page *page, uintptr linear_address, uint32 perm)
{
    pte *entry = get_page_table_entry(page_dir, linear_address, true);
    if (entry == NULL)
    {
        return -E_NO_MEM;
    }

    increase_page_reference(page);
    if (*entry & PTE_P)
    {
        struct Page *p = page_table_entry_to_page(*entry);
        if (p == page)
        {
            decrease_page_reference(page);
        }
        else
        {
            remove_page_and_page_table_entry(page_dir, linear_address, entry);
        }
    }
    *entry = page_to_physical_address(page) | PTE_P | perm;
    tlb_invalidate(page_dir, linear_address);
    return 0;
}

// 删除页对应的页表项，并将对应的Page
void page_remove(pde *page_dir, uintptr linear_address)
{
    pte *entry = get_page_table_entry(page_dir, linear_address, 0);
    if (entry != NULL)
    {
        remove_page_and_page_table_entry(page_dir, linear_address, entry);
    }
}

struct Page *page_dir_alloc_page(pde *page_dir, uintptr linear_address, uint32 perm)
{
    struct Page *page = allocate_pages(1);
    if (page != NULL)
    {
        if (page_insert(page_dir, page, linear_address, perm) != 0)
        {
            deallocate_pages(page, 1);
            return NULL;
        }
//        if (swap_init_ok && virtual_memory_verification != NULL)
//        {
//            swap_map_swappable(virtual_memory_verification, linear_address, page, 0);
//            page->pra_vaddr = linear_address;
//            assert(get_page_reference(page) == 1);
//            //kernel_print("get No. %d  page: pra_vaddr %x, pra_link.prev %x, pra_link_next %x in pgdir_alloc_page\n", (page-pages), page->pra_vaddr,page->pra_page_link.prev, page->pra_page_link.next);
//        }
    }
    return page;
}

void print_page_table_item(uintptr address)
{
    kernel_print("\n======== print_page_table_item() ========\n\n");
    pde *item = (pde *) address;
    for (int i = 0; i < 1024; i++)
    {
        if (*(item + i) == 0)
            continue;
        kernel_print("pde item = %04d: 0x%x\n", i, PageDirectoryEntryAddress(*(item + i)));
        pte* entry = (pte*)(VirtualAddress(PageTableEntryAddress(*(item + i))));
        for (int j = 0; j < 1024; j++)
        {
            if (*(entry + j) == 0 || (*(entry + j) & (PTE_U | PTE_P)) != (PTE_U | PTE_P))
                continue;

            kernel_print(" |--pte item = %04d: 0x%x\n", j, PageTableEntryAddress(*(entry + j)));
        }
    }
    kernel_print("\n======== print_page_table_item() ========\n\n");
}
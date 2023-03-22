#include "physical.h"
#include "gdt.h"
#include <x86.h>
#include <stdio.h>
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

extern char boot_stack[], boot_stack_top[];
extern pde __boot_page_dir;
pde *boot_page_dir = &__boot_page_dir;
uintptr boot_cr3;

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

static void load_esp0(uintptr esp0)
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
    printf("memory management: %s\n", memory_manager->name);
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

    printf("e820map: %d\n", memory_map->num);
    int i;
    for (i = 0; i < memory_map->num; i++)
    {
        start = memory_map->map[i].address;
        finish = start + memory_map->map[i].size;
        printf("  memory: size = %08llx, [%08llx, %08llx], type = %d.\n",
               memory_map->map[i].size, start, finish - 1, memory_map->map[i].type);

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
    printf("max physical address = %08llx, num of physical page = %d, end = %p\n", max_physical_address, num_of_physical_page, end);

    // 从end位置开始（按4096对齐后），分配Page结构体，第一个Page结构体永远对应第一个物理页，依次类推
    // 将内核所占的空间 KERNEL_MEMORY_SIZE， 对应的Page设置为已使用
    pages = (struct Page *) RoundUp((void *) end, PAGE_SIZE);
    for (i = 0; i < num_of_physical_page; i++)
    {
        SetPageReserved(pages + i);
    }

    uintptr freemem = PageAddress((uintptr) pages + sizeof(struct Page) * num_of_physical_page);
    printf("freemem = %p\n", freemem);

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
                printf("[ start = %08llx, finish = %08llx ]\n", start, finish);
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


void physical_memory_init()
{
    boot_cr3 = PageAddress(boot_page_dir);

    physical_memory_manager_init();

    physical_page_init();

    gdt_init();
}
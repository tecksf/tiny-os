#include "physical.h"
#include "page.h"
#include "gdt.h"
#include <x86.h>

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

void physical_memory_init()
{
    gdt_init();
}
// 可编程中断控制器 Programmable Interrupt Controller

#include "picirq.h"
#include <defs.h>
#include <x86.h>

#define IO_PIC_MASTER 0x20 // 主控制器 (IRQs 0-7)
#define IO_PIC_SLAVE 0xA0 // 从控制器 (IRQs 8-15)

#define IRQ_SLAVE 2

static uint16 irq_mask = 0xFFFF & ~(1 << IRQ_SLAVE);
static bool did_init = 0;

static void pic_set_mask(uint16 mask)
{
    irq_mask = mask;
    if (did_init)
    {
        outb(IO_PIC_MASTER + 1, mask);
        outb(IO_PIC_SLAVE + 1, mask >> 8);
    }
}


// 设置FFLAGS的中断标志位IF，即开启或关闭CPU的中断功能
void cpu_interrupt_enable(void)
{
    sti();
}

void cpu_interrupt_disable(void)
{
    cli();
}

// 将OCW1对应位置为0,即开启IRQ号中断，irq取值为0～15
void pic_enable(uint32 irq)
{
    pic_set_mask(irq_mask & ~(1 << irq));
}

// 初始化 8259A 中断控制器
void pic_init(void)
{
    did_init = 1;

    // 初始化之前，屏蔽所有的中断，因此将所有IRQ对应的OCW1的位置为1
    outb(IO_PIC_MASTER + 1, 0xFF);
    outb(IO_PIC_SLAVE + 1, 0xFF);

    // 初始化主片
    // LTIM = 0,边沿触发; SNGL = 0,表示级联; IC4 = 1,表示需要写入ICW4
    outb(IO_PIC_MASTER, 0x11);

    // ICW2 设置主片起始中断向量号 IRQ0则为32
    outb(IO_PIC_MASTER + 1, IRQ_OFFSET);

    // ICW3 设置主片的IRQ2为级联接口
    outb(IO_PIC_MASTER + 1, 1 << IRQ_SLAVE);

    // ICW4 设置自动发结束中断
    outb(IO_PIC_MASTER + 1, 0x3);


    // 初始化从片 ICW1
    outb(IO_PIC_SLAVE, 0x11);

    // ICW2 设置从片起始中断向量号 IRQ8则为40
    outb(IO_PIC_SLAVE + 1, IRQ_OFFSET + 8);

    // ICW3 主片接口为IRQ2，从片ICW3的值为0x2
    outb(IO_PIC_SLAVE + 1, IRQ_SLAVE);

    // ICW4 设置自动发结束中断
    outb(IO_PIC_SLAVE + 1, 0x3);


    // OCW3
    outb(IO_PIC_MASTER, 0x68); // clear specific mask
    outb(IO_PIC_MASTER, 0x0a); // read IRR by default

    outb(IO_PIC_SLAVE, 0x68); // OCW3
    outb(IO_PIC_SLAVE, 0x0a); // OCW3

    // 因为主片的IRQ2与从片级联，因此将ICW1中对应的第2位置为0,
    // 主片 1111 1011，从片1111 1111
    if (irq_mask != 0xFFFF)
    {
        pic_set_mask(irq_mask);
    }
}

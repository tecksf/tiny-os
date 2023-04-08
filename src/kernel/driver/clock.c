#include "clock.h"
#include "console.h"
#include <picirq.h>
#include <x86.h>
#include <trap.h>

#define IO_TIMER1       0x040           // 计数器0的端口

#define TIMER_FREQ      1193182
#define TIMER_DIV(x)    ((TIMER_FREQ + (x) / 2) / (x))

#define TIMER_MODE      (IO_TIMER1 + 3) //控制寄存器端口
#define TIMER_SEL0      0x00            // 选择计数器0
#define TIMER_RATEGEN   0x04            // 计数器工作方式2
#define TIMER_16BIT     0x30            // 先读写低字节，后读写高字节

volatile usize ticks;

void clock_init(void)
{
    // 设置8253的工作方式
    outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);

    outb(IO_TIMER1, TIMER_DIV(100) % 256);
    outb(IO_TIMER1, TIMER_DIV(100) / 256);

    ticks = 0;

    kernel_print("============== setup timer interrupts ==============\n");

    // 开启PIC的IRQ0的中断，即时钟中断
    pic_enable(IRQ_TIMER);
}

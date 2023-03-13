#include <x86.h>
#include <gdt.h>
#include <clock.h>
#include <keyboard.h>
#include <stdio.h>
#include "picirq.h"
#include "trap.h"
#include "gate.h"

static struct GateDescriptor idt[256] = {{0}};
static struct pseudodesc idt_pd = { sizeof(idt) - 1, (uintptr)idt };

void idt_init()
{
    extern uint32 __vectors[];

    int i;
    for (i = 0; i < sizeof(idt) / sizeof(struct GateDescriptor); i++)
    {
        SetGate(idt[i], 0, GD_KTEXT, __vectors[i], DPL_KERNEL);
    }

    // 将IDT地址和大小加载到IDTR寄存器中
    lidt(&idt_pd);
}

static void trap_dispatch(struct TrapFrame *tf)
{
    uint8 rc;

    switch (tf->tf_trapno)
    {
        case IRQ_OFFSET + IRQ_TIMER:
            ticks++;
            if (ticks % TICK_NUM == 0)
                printf("interrupt: %d ticks\n", ticks / TICK_NUM);
            break;
        case IRQ_OFFSET + IRQ_KBD:
            rc = keyboard_interrupt();
            // 松开按键返回0
            if (rc != 0)
                put_char(rc);
            break;
        default:
    }
}

void trap(struct TrapFrame *tf)
{
    trap_dispatch(tf);
}
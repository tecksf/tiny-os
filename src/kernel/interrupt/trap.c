#include <x86.h>
#include <gdt.h>
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

void trap(struct TrapFrame *tf)
{

}
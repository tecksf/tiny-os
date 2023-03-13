#ifndef __INTERRUPT_TRAP_H__
#define __INTERRUPT_TRAP_H__

#include <defs.h>

#define T_DIVIDE        0    // divide error
#define T_DEBUG         1    // debug exception
#define T_NMI           2    // non-maskable interrupt
#define T_BRKPT         3    // breakpoint
#define T_OFLOW         4    // overflow
#define T_BOUND         5    // bounds check
#define T_ILLOP         6    // illegal opcode
#define T_DEVICE        7    // device not available
#define T_DBLFLT        8    // double fault
#define T_COPROC        9    // reserved (not used since 486)
#define T_TSS           10   // invalid task switch segment
#define T_SEGNP         11   // segment not present
#define T_STACK         12   // stack exception
#define T_GPFLT         13   // general protection fault
#define T_PGFLT         14   // page fault
#define T_RES           15   // reserved
#define T_FPERR         16   // floating point error
#define T_ALIGN         17   // aligment check
#define T_MCHK          18   // machine check
#define T_SIMDERR       19   // SIMD floating point error

#define IRQ_TIMER       0
#define IRQ_KBD         1
#define IRQ_COM1        4
#define IRQ_IDE1        14
#define IRQ_IDE2        15
#define IRQ_ERROR       19
#define IRQ_SPURIOUS    31

struct PushRegs
{
    uint32 reg_edi;
    uint32 reg_esi;
    uint32 reg_ebp;
    uint32 reg_oesp;
    uint32 reg_ebx;
    uint32 reg_edx;
    uint32 reg_ecx;
    uint32 reg_eax;
} __attribute__((packed));

struct TrapFrame
{
    struct PushRegs tf_regs;
    uint16 tf_gs;
    uint16 tf_padding0;
    uint16 tf_fs;
    uint16 tf_padding1;
    uint16 tf_es;
    uint16 tf_padding2;
    uint16 tf_ds;
    uint16 tf_padding3;
    uint32 tf_trapno;
    uint32 tf_err;
    uintptr tf_eip;
    uint16 tf_cs;
    uint16 tf_padding4;
    uint32 tf_eflags;
    uintptr tf_esp;
    uint16 tf_ss;
    uint16 tf_padding5;
} __attribute__((packed));

void idt_init();
void trap(struct TrapFrame *tf);

#endif // __INTERRUPT_TRAP_H__

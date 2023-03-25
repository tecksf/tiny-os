#include <x86.h>
#include <gdt.h>
#include <clock.h>
#include <keyboard.h>
#include <stdio.h>
#include <assert.h>
#include <env.h>
#include "virtual.h"
#include "picirq.h"
#include "trap.h"
#include "gate.h"

static struct GateDescriptor idt[256] = {{0}};
static struct pseudodesc idt_pd = {sizeof(idt) - 1, (uintptr) idt};
static const char *IA32flags[] = {
        "CF", NULL, "PF", NULL, "AF", NULL, "ZF", "SF",
        "TF", "IF", "DF", "OF", NULL, NULL, "NT", NULL,
        "RF", "VM", "AC", "VIF", "VIP", "ID", NULL, NULL,
};

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

static const char *get_trap_name(int trap_number)
{
    static const char *const names[] = {
            "Divide error",
            "Debug",
            "Non-Maskable Interrupt",
            "Breakpoint",
            "Overflow",
            "BOUND Range Exceeded",
            "Invalid Opcode",
            "Device Not Available",
            "Double Fault",
            "Coprocessor Segment Overrun",
            "Invalid TSS",
            "Segment Not Present",
            "Stack Fault",
            "General Protection",
            "Page Fault",
            "(unknown trap)",
            "x87 FPU Floating-Point Error",
            "Alignment Check",
            "Machine-Check",
            "SIMD Floating-Point Exception"};

    if (trap_number < sizeof(names) / sizeof(const char *const))
    {
        return names[trap_number];
    }

    if (trap_number >= IRQ_OFFSET && trap_number < IRQ_OFFSET + 16)
    {
        return "Hardware Interrupt";
    }
    return "(unknown trap)";
}

/* trap_in_kernel - test if trap happened in kernel */
bool trap_in_kernel(struct TrapFrame *tf)
{
    return (tf->tf_cs == (uint16) KERNEL_CS);
}

void print_regs(struct PushRegs *regs)
{
    printf("  edi  0x%08x\n", regs->reg_edi);
    printf("  esi  0x%08x\n", regs->reg_esi);
    printf("  ebp  0x%08x\n", regs->reg_ebp);
    printf("  oesp 0x%08x\n", regs->reg_oesp);
    printf("  ebx  0x%08x\n", regs->reg_ebx);
    printf("  edx  0x%08x\n", regs->reg_edx);
    printf("  ecx  0x%08x\n", regs->reg_ecx);
    printf("  eax  0x%08x\n", regs->reg_eax);
}

void print_trap_frame(struct TrapFrame *tf)
{
    printf("trap frame at %p\n", tf);
    print_regs(&tf->tf_regs);
    printf("  ds   0x----%04x\n", tf->tf_ds);
    printf("  es   0x----%04x\n", tf->tf_es);
    printf("  fs   0x----%04x\n", tf->tf_fs);
    printf("  gs   0x----%04x\n", tf->tf_gs);
    printf("  trap 0x%08x %s\n", tf->tf_trapno, get_trap_name(tf->tf_trapno));
    printf("  err  0x%08x\n", tf->tf_err);
    printf("  eip  0x%08x\n", tf->tf_eip);
    printf("  cs   0x----%04x\n", tf->tf_cs);
    printf("  flag 0x%08x ", tf->tf_eflags);

    int i, j;
    for (i = 0, j = 1; i < sizeof(IA32flags) / sizeof(IA32flags[0]); i++, j <<= 1)
    {
        if ((tf->tf_eflags & j) && IA32flags[i] != NULL)
        {
            printf("%s,", IA32flags[i]);
        }
    }
    printf("IOPL=%d\n", (tf->tf_eflags & FL_IOPL_MASK) >> 12);

    if (!trap_in_kernel(tf))
    {
        printf("  esp  0x%08x\n", tf->tf_esp);
        printf("  ss   0x----%04x\n", tf->tf_ss);
    }
}

static inline void print_page_fault(struct TrapFrame *tf)
{
    /* error_code:
     * bit 0 == 0 means no page found, 1 means protection fault
     * bit 1 == 0 means read, 1 means write
     * bit 2 == 0 means kernel, 1 means user
     * */
    printf("page fault at 0x%08x: %c/%c [%s].\n", rcr2(),
           (tf->tf_err & 4) ? 'U' : 'K',
           (tf->tf_err & 2) ? 'W' : 'R',
           (tf->tf_err & 1) ? "protection fault" : "no page found");
}

static int page_fault_handler(struct TrapFrame *tf)
{
    print_page_fault(tf);
    if (virtual_memory_verification != NULL)
    {
        return do_page_fault(virtual_memory_verification, tf->tf_err, rcr2());
    }
    panic("unhandled page fault.\n");
}

static void trap_dispatch(struct TrapFrame *tf)
{
    uint8 rc;

    switch (tf->tf_trapno)
    {
        case T_PGFLT:
            if ((rc = page_fault_handler(tf)) != 0)
            {
                print_trap_frame(tf);
                panic("handle page fault failed. %e\n", rc);
            }
            break;
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
        case IRQ_OFFSET + IRQ_IDE1:
        case IRQ_OFFSET + IRQ_IDE2:
            /* do nothing */
            break;
        default:
            printf("default trap dispatch, trap number = %d\n", tf->tf_trapno);
            break;
    }
}

void trap(struct TrapFrame *tf)
{
    trap_dispatch(tf);
}
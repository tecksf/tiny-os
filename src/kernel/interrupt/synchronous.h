#ifndef __INTERRUPT_SYNCHRONOUS_H__
#define __INTERRUPT_SYNCHRONOUS_H__

#include <x86.h>
#include <gdt.h>
#include "picirq.h"

static inline bool __interrupt_save(void)
{
    if (read_eflags() & FL_IF)
    {
        cpu_interrupt_disable();
        return 1;
    }
    return 0;
}

static inline void __interrupt_restore(bool flag)
{
    if (flag)
    {
        cpu_interrupt_enable();
    }
}

#define SaveLocalInterrupt(x)      do { x = __interrupt_save(); } while (0)
#define RestoreLocalInterrupt(x)   __interrupt_restore(x)

#endif // __INTERRUPT_SYNCHRONOUS_H__

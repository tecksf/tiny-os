//
// Created by tang on 3/12/23.
//

#include "core/env.h"
#include "driver/console.h"
#include "driver/clock.h"
#include "driver/keyboard.h"
#include "driver/ide.h"
#include "interrupt/picirq.h"
#include "interrupt/trap.h"
#include "memory/physical.h"
#include "memory/virtual.h"
#include "filesystem/swap.h"
#include "debug/kernel_print.h"
#include "process/process.h"
#include "process/load.h"
#include <string.h>

static inline void kernel_bss_segment_init()
{
    extern char edata[], end[];
    memory_set(edata, 0, end - edata);
}

void kernel_init(void)
{
    kernel_bss_segment_init();

    cons_init();

    const char *message = "os is loading ...";
    kernel_print("%s\n\n", message);
    kernel_puts("start to initialize IDT...");
    print_kernel_info();

    physical_memory_init();

    pic_init();
    idt_init();

    virtual_memory_init();

    ide_init();
    swap_init();

    load_program();

    process_init();

//    clock_init();

    keyboard_init();

    cpu_interrupt_enable();

    cpu_idle();
}
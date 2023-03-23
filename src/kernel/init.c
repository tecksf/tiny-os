//
// Created by tang on 3/12/23.
//
#include <stdio.h>
#include "driver/console.h"
#include "driver/clock.h"
#include "driver/keyboard.h"
#include "interrupt/picirq.h"
#include "interrupt/trap.h"
#include "memory/physical.h"
#include "debug/kernel_print.h"

void kernel_init(void)
{
    cons_init();

    const char *message = "os is loading ...";
    printf("%s\n\n", message);
    puts("start to initialize IDT...");
    print_kernel_info();

    physical_memory_init();

    pic_init();
    idt_init();

//    clock_init();

    keyboard_init();

    cpu_interrupt_enable();

    while (1);
}
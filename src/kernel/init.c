//
// Created by tang on 3/12/23.
//
#include <stdio.h>
#include "driver/console.h"
#include "interrupt/picirq.h"

void kern_init(void)
{
    cons_init();

    const char *message = "os is loading ...";
    printf("%s\n\n", message);

    puts("start to initialize IDT...");
    pic_init();
}
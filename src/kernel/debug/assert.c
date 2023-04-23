#include "assert.h"
#include "kernel_print.h"
#include <console.h>
#include <stdarg.h>
#include <picirq.h>

static bool is_panic = false;

void __panic(const char *file, int line, const char *fmt, ...)
{
    if (is_panic)
    {
        goto panic_dead;
    }
    is_panic = true;

    va_list ap;
    va_start(ap, fmt);
    kernel_print("kernel panic at %s:%d:\n    ", file, line);
    variant_print(fmt, ap);
    kernel_print("\n");

    kernel_print("stack track back:\n");
    print_stack_frame();

    va_end(ap);

panic_dead:
    cpu_interrupt_disable();
    while (1);
}

void __warn(const char *file, int line, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    kernel_print("kernel warning at %s:%d:\n    ", file, line);
    variant_print(fmt, ap);
    kernel_print("\n");
    va_end(ap);
}

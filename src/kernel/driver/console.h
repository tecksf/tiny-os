#ifndef __DRIVER_CONSOLE_H__
#define __DRIVER_CONSOLE_H__

#include <stdarg.h>

void cons_init(void);
void cons_putc(int c);

int kernel_puts(const char *str);
int kernel_print(const char *fmt, ...);
void kernel_put_char(int c);
int variant_print(const char *fmt, va_list ap);

#endif // __DRIVER_CONSOLE_H__

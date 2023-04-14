#include "stdio.h"
#include "syscall.h"
#include <format.h>
#include <stdarg.h>

static void inner_put_char(int c, int *cnt)
{
    system_put_char(c);
    (*cnt)++;
}

int printf(const char *fmt, ...)
{
    va_list ap;
    int cnt = 0;

    va_start(ap, fmt);
    format_output((void *) inner_put_char, &cnt, fmt, ap);
    va_end(ap);

    return cnt;
}

int puts(const char *str)
{
    int cnt = 0;
    char c;
    while ((c = *str++) != '\0')
    {
        inner_put_char(c, &cnt);
    }
    inner_put_char('\n', &cnt);
    return cnt;
}
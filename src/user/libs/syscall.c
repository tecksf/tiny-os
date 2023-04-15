#include "syscall.h"
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <trap.h>

#define MAX_ARGS 5


static inline int syscall(int num, ...)
{
    va_list ap;
    va_start(ap, num);
    uint32 a[MAX_ARGS];
    int i, rc;
    for (i = 0; i < MAX_ARGS; i++)
    {
        a[i] = va_arg(ap, uint32);
    }
    va_end(ap);

    asm volatile(
            "int %1;"
            : "=a"(rc)
            : "i"(T_SYSCALL),
    "a"(num),
    "d"(a[0]),
    "c"(a[1]),
    "b"(a[2]),
    "D"(a[3]),
    "S"(a[4])
            : "cc", "memory");
    return rc;
}

void exit(int error_code)
{
    syscall(SYS_EXIT, error_code);
    printf("BUG: exit failed.\n");
    while (1);
}

int fork(void)
{
    return syscall(SYS_FORK);
}

int wait(void)
{
    return syscall(SYS_WAIT, 0, NULL);
}

int waitpid(int pid, int *store)
{
    return syscall(SYS_WAIT, pid, store);
}

void yield(void)
{
    syscall(SYS_YIELD);
}

int kill(int pid)
{
    return syscall(SYS_KILL, pid);
}

int getpid(void)
{
    return syscall(SYS_GET_PID);
}

int system_put_char(int c)
{
    return syscall(SYS_PUTC, c);
}

int system_page_dir(void)
{
    return syscall(SYS_PGDIR);
}

#include "syscall.h"
#include <defs.h>
#include <stdarg.h>

#define MAX_ARGS 5

#define T_SYSCALL           0x80

#define SYS_EXIT            1
#define SYS_FORK            2
#define SYS_WAIT            3
#define SYS_EXEC            4
#define SYS_CLONE           5
#define SYS_YIELD           10
#define SYS_SLEEP           11
#define SYS_KILL            12
#define SYS_GET_TIME        17
#define SYS_GET_PID         18
#define SYS_BRK             19
#define SYS_MMAP            20
#define SYS_MUNMAP          21
#define SYS_SHMEM           22
#define SYS_PUTC            30
#define SYS_PGDIR           31

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

int system_exit(int error_code)
{
    return syscall(SYS_EXIT, error_code);
}

int system_fork(void)
{
    return syscall(SYS_FORK);
}

int system_wait(int pid, int *store)
{
    return syscall(SYS_WAIT, pid, store);
}

int system_yield(void)
{
    return syscall(SYS_YIELD);
}

int system_kill(int pid)
{
    return syscall(SYS_KILL, pid);
}

int system_get_pid(void)
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

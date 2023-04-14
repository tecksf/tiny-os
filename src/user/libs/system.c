#include "system.h"
#include "syscall.h"
#include <stdio.h>

void exit(int error_code)
{
    system_exit(error_code);
    printf("BUG: exit failed.\n");
    while (1);
}

int fork(void)
{
    return system_fork();
}

int wait(void)
{
    return system_wait(0, NULL);
}

int waitpid(int pid, int *store)
{
    return system_wait(pid, store);
}

void yield(void)
{
    system_yield();
}

int kill(int pid)
{
    return system_kill(pid);
}

int getpid(void)
{
    return system_get_pid();
}

#include "system.h"
#include "process.h"
#include <console.h>
#include <unistd.h>
#include <assert.h>

static int system_exit(uint32 arg[])
{
    int error_code = (int) arg[0];
    return do_exit(error_code);
}

static int system_fork(uint32 arg[])
{
    struct TrapFrame *tf = current_process->tf;
    uintptr stack = tf->tf_esp;
    return do_fork(0, stack, tf);
}

static int system_wait(uint32 arg[])
{
    int pid = (int) arg[0];
    int *store = (int *) arg[1];
    return do_wait(pid, store);
}

static int system_exec(uint32 arg[])
{
    const char *name = (const char *) arg[0];
    usize len = (usize) arg[1];
    unsigned char *binary = (unsigned char *) arg[2];
    usize size = (usize) arg[3];
    return do_execve(name, len, binary, size);
}

static int system_yield(uint32 arg[])
{
    return do_yield();
}

static int system_kill(uint32 arg[])
{
    int pid = (int) arg[0];
    return do_kill(pid);
}

static int system_getpid(uint32 arg[])
{
    return current_process->pid;
}

static int system_put_char(uint32 arg[])
{
    int c = (int) arg[0];
    kernel_put_char(c);
    return 0;
}

static int system_pgdir(uint32 arg[])
{
    // print_page_dir();
    return 0;
}

static int (*syscall_table[])(uint32 arg[]) = {
        [SYS_EXIT]                  system_exit,
        [SYS_FORK]                  system_fork,
        [SYS_WAIT]                  system_wait,
        [SYS_EXEC]                  system_exec,
        [SYS_YIELD]                 system_yield,
        [SYS_KILL]                  system_kill,
        [SYS_GET_PID]               system_getpid,
        [SYS_PUTC]                  system_put_char,
        [SYS_PGDIR]                 system_pgdir,
};

#define NUM_SYSTEM_CALLS ((sizeof(syscall_table)) / (sizeof(syscall_table[0])))

void system_execute(struct TrapFrame* tf)
{
    uint32 arg[5];
    int num = tf->tf_regs.reg_eax;
    if (num >= 0 && num < NUM_SYSTEM_CALLS)
    {
        if (syscall_table[num] != NULL)
        {
            arg[0] = tf->tf_regs.reg_edx;
            arg[1] = tf->tf_regs.reg_ecx;
            arg[2] = tf->tf_regs.reg_ebx;
            arg[3] = tf->tf_regs.reg_edi;
            arg[4] = tf->tf_regs.reg_esi;
            tf->tf_regs.reg_eax = syscall_table[num](arg);
            return;
        }
    }
    print_trap_frame(tf);
    panic("undefined syscall %d, pid = %d, name = %s.\n", num, current_process->pid, current_process->name);
}
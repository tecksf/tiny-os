#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <defs.h>
#include <list.h>
#include <trap.h>
#include <virtual.h>

#define PROCESS_NAME_LEN            15
#define MAX_PROCESS                 4096
#define MAX_PID                     (MAX_PROCESS * 2)

#define PF_EXITING                  0x00000001      // getting shutdown

#define WT_INTERRUPTED              0x80000000                    // the wait state could be interrupted
#define WT_CHILD                    (0x00000001 | WT_INTERRUPTED)


#define OffsetOfProcessControlBlock(le, member) container_of((le), struct ProcessControlBlock, member)

enum ProcessState
{
    PROCESS_UNINITIALIZED = 0,      // uninitialized
    PROCESS_SLEEPING,               // sleeping
    PROCESS_RUNNABLE,               // runnable(maybe running)
    PROCESS_ZOMBIE,                 // almost dead, and wait parent proc to reclaim his resource
};

struct Context
{
    uint32 eip;
    uint32 esp;
    uint32 ebx;
    uint32 ecx;
    uint32 edx;
    uint32 esi;
    uint32 edi;
    uint32 ebp;
};

// 进程控制快PCB
struct ProcessControlBlock
{
    enum ProcessState state;            // 进程的状态
    int pid;                            // 进程号
    int runs;                           // 进程运行次数，即被调度的次数
    uintptr stack;                      // 进程栈
    volatile bool need_reschedule;      // bool value: need to be rescheduled to release CPU?
    struct ProcessControlBlock *parent; // 进程的父进程
    struct VirtualMemory *memory;       // 用户态进程的内存空间管理
    struct Context context;             // 进程的上下文，用于进程切换
    struct TrapFrame *tf;               // 中断帧指针，进程从用户态跳到内核空间时，需要保存当前状态
    uintptr cr3;                        // 存储页目录表的物理地址
    uint32 flags;                       // 进程flag
    char name[PROCESS_NAME_LEN + 1];    // 进程名字
    ListEntry list_link;       // Process link list
    ListEntry hash_link;       // Process hash list
    int exit_code;                                      // 当前进程退出时的原因，会由父进程获取
    uint32 wait_state;                                  // 当前进程进入等待的原因
    struct ProcessControlBlock *cptr, *yptr, *optr;     // relations between processes
};

extern struct ProcessControlBlock *idle_process;
extern struct ProcessControlBlock *init_process;
extern struct ProcessControlBlock *current_process;
extern ListEntry process_list;

void process_init(void);
void run_process(struct ProcessControlBlock *process);
void cpu_idle(void) __attribute__((noreturn));

char *set_process_name(struct ProcessControlBlock *process, const char *name);
char *get_process_name(struct ProcessControlBlock *process);

int do_fork(uint32 clone_flags, uintptr stack, struct TrapFrame *tf);
int do_exit(int error_code);
int do_yield(void);
int do_execve(const char *name, usize name_length, unsigned char *binary, usize binary_size);
int do_wait(int pid, int *code_store);
int do_kill(int pid);

#endif // __PROCESS_H__

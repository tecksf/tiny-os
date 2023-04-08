#include "process.h"
#include "schedule.h"
#include <stdlib.h>
#include <env.h>
#include <gdt.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <assert.h>
#include <physical.h>
#include <synchronous.h>

#define CLONE_VM            0x00000100  // set if VM shared between processes
#define CLONE_THREAD        0x00000200  // thread group

#define HASH_SHIFT 10
#define HASH_LIST_SIZE (1 << HASH_SHIFT)
#define pid_hash_fn(x) (hash32(x, HASH_SHIFT))

ListEntry process_list;

// 进程控制快的hash表，以pid位key;有重复的散列值以双向链表形式组织
static ListEntry hash_list[HASH_LIST_SIZE];
static int num_process = 0;

struct ProcessControlBlock *idle_process = NULL;
struct ProcessControlBlock *init_process = NULL;
struct ProcessControlBlock *current_process = NULL;

void kernel_thread_entry(void);
void switch_to(struct Context *from, struct Context *to);
void fork_rets(struct TrapFrame *tf);

static int init_main(void *arg)
{
    printf("this init process, pid = %d, name = \"%s\"\n", current_process->pid, get_process_name(current_process));
    printf("To U: \"%s\".\n", (const char *) arg);
    printf("To U: \"en.., Bye, Bye. :)\"\n");
    return 0;
}

static struct ProcessControlBlock *create_process(void)
{
    struct ProcessControlBlock *process = kernel_malloc(sizeof(struct ProcessControlBlock));
    if (process != NULL)
    {
        process->state = PROCESS_UNINITIALIZED;
        process->pid = -1;
        process->runs = 0;
        process->stack = 0;
        process->need_reschedule = 0;
        process->parent = NULL;
        process->memory = NULL;
        memory_set(&(process->context), 0, sizeof(struct Context));
        process->tf = NULL;
        process->cr3 = boot_cr3;
        process->flags = 0;
        memory_set(process->name, 0, PROCESS_NAME_LEN);
    }
    return process;
}

static int get_pid(void)
{
    static_assert(MAX_PID > MAX_PROCESS)

    struct ProcessControlBlock *process;
    ListEntry *list = &process_list, *le;
    static int next_safe = MAX_PID, last_pid = MAX_PID;

    if (++last_pid >= MAX_PID)
    {
        last_pid = 1;
        goto inside;
    }

    if (last_pid >= next_safe)
    {
inside:
        next_safe = MAX_PID;
repeat:
        le = list;
        while ((le = list_next(le)) != list)
        {
            process = OffsetOfProcessControlBlock(le, list_link);
            if (process->pid == last_pid)
            {
                if (++last_pid >= next_safe)
                {
                    if (last_pid >= MAX_PID)
                    {
                        last_pid = 1;
                    }
                    next_safe = MAX_PID;
                    goto repeat;
                }
            }
            else if (process->pid > last_pid && next_safe > process->pid)
            {
                next_safe = process->pid;
            }
        }
    }
    return last_pid;
}

// 分配一个进程使用的栈，每个进程都不同
static int setup_kernel_stack(struct ProcessControlBlock *process)
{
    struct Page *page = allocate_pages(KERNEL_STACK_PAGE_COUNT);
    if (page != NULL)
    {
        process->stack = (uintptr) page_to_virtual_address(page);
        return 0;
    }
    return -E_NO_MEM;
}

static void release_kernel_stack(struct ProcessControlBlock *process)
{
//    free_pages(virtual_address_to_page((void *)(process->stack)), KERNEL_STACK_PAGE_COUNT);
}

static int copy_memory(uint32 clone_flags, struct ProcessControlBlock *process)
{
    assert(current_process->memory == NULL);
    /* do nothing in this project */
    return 0;
}

static void fork_ret(void)
{
    fork_rets(current_process->tf);
}

static void hash_process(struct ProcessControlBlock *process)
{
    list_add(hash_list + pid_hash_fn(process->pid), &(process->hash_link));
}

// 设置进程在内核正常运行和调度所需的中断帧和执行上下文
static void copy_thread(struct ProcessControlBlock *process, uintptr esp, struct TrapFrame *tf)
{
    process->tf = (struct TrapFrame *) (process->stack + KERNEL_STACK_SIZE) - 1;
    *(process->tf) = *tf;
    process->tf->tf_regs.reg_eax = 0;
    process->tf->tf_esp = esp;
    process->tf->tf_eflags |= FL_IF;

    process->context.eip = (uintptr) fork_ret;
    process->context.esp = (uintptr) (process->tf);
}

void run_process(struct ProcessControlBlock *process)
{
    if (process != current_process)
    {
        bool flag;
        struct ProcessControlBlock *prev = current_process, *next = process;
        SaveLocalInterrupt(flag);
        {
            current_process = process;
            load_esp0(next->stack + KERNEL_STACK_SIZE);
            lcr3(next->cr3);
            switch_to(&(prev->context), &(next->context));
        }
        RestoreLocalInterrupt(flag);
    }
}

struct ProcessControlBlock *find_process(int pid)
{
    if (0 < pid && pid < MAX_PID)
    {
        ListEntry *list = hash_list + pid_hash_fn(pid), *le = list;
        while ((le = list_next(le)) != list)
        {
            struct ProcessControlBlock *process = OffsetOfProcessControlBlock(le, hash_link);
            if (process->pid == pid)
            {
                return process;
            }
        }
    }
    return NULL;
}

int kernel_thread(int (*fn)(void *), void *arg, uint32 clone_flags)
{
    struct TrapFrame tf;
    memory_set(&tf, 0, sizeof(struct TrapFrame));
    tf.tf_cs = KERNEL_CS;
    tf.tf_ds = tf.tf_es = tf.tf_ss = KERNEL_DS;
    tf.tf_regs.reg_ebx = (uint32) fn;
    tf.tf_regs.reg_edx = (uint32) arg;
    tf.tf_eip = (uint32) kernel_thread_entry;
    return do_fork(clone_flags | CLONE_VM, 0, &tf);
}

char *set_process_name(struct ProcessControlBlock *process, const char *name)
{
    memory_set(process->name, 0, sizeof(process->name));
    return memory_copy(process->name, name, PROCESS_NAME_LEN);
}

char *get_process_name(struct ProcessControlBlock *process)
{
    static char name[PROCESS_NAME_LEN + 1];
    memory_set(name, 0, sizeof(name));
    return memory_copy(name, process->name, PROCESS_NAME_LEN);
}

void process_init(void)
{
    num_process = 0;
    list_init(&process_list);
    for (int i = 0; i < HASH_LIST_SIZE; i++)
    {
        list_init(hash_list + i);
    }

    if ((idle_process = create_process()) == NULL)
    {
        panic("cannot alloc idle process.\n");
    }

    idle_process->pid = 0;
    idle_process->state = PROCESS_RUNNABLE;
    idle_process->stack = (uintptr) boot_stack;
    idle_process->need_reschedule = 1;
    set_process_name(idle_process, "idle");
    num_process++;

    current_process = idle_process;

    int pid = kernel_thread(init_main, "Hello world!!", 0);
    if (pid <= 0)
    {
        panic("create init_main failed.\n");
    }

    init_process = find_process(pid);
    set_process_name(init_process, "init");

    assert(idle_process != NULL && idle_process->pid == 0);
    assert(init_process != NULL && init_process->pid == 1);
}

void cpu_idle(void)
{
    while (1)
    {
        if (current_process->need_reschedule)
        {
            schedule();
        }
    }
}

// 创建一个新的进程
int do_fork(uint32 clone_flags, uintptr stack, struct TrapFrame *tf)
{
    int rc = -E_NO_FREE_PROC;
    struct ProcessControlBlock *process;
    if (num_process >= MAX_PROCESS)
    {
        goto fork_out;
    }
    rc = -E_NO_MEM;

    // 申请一个进程控制块
    if ((process = create_process()) == NULL)
    {
        goto fork_out;
    }

    // 当前进程为新进程的父进程
    process->parent = current_process;

    // 为新进程设置栈空间
    if (setup_kernel_stack(process) != 0)
    {
        goto bad_fork_cleanup_process;
    }

    // 拷贝当前进程的内存空间到新进程，clone_flags决定以共享或复制形式拷贝
    if (copy_memory(clone_flags, process) != 0)
    {
        goto bad_fork_cleanup_kernel_stack;
    }
    copy_thread(process, stack, tf);

    // 将新的进程控制快放入全局的proc_list和hash_list
    bool flag;
    SaveLocalInterrupt(flag);
    {
        process->pid = get_pid();
        hash_process(process);
        list_add(&process_list, &(process->list_link));
        num_process++;
    }
    RestoreLocalInterrupt(flag);

    wakeup_process(process);

    rc = process->pid;
fork_out:
    return rc;

bad_fork_cleanup_kernel_stack:
    release_kernel_stack(process);
bad_fork_cleanup_process:
    // kfree(proc);
    goto fork_out;
}

int do_exit(int error_code)
{
    return 0;
}

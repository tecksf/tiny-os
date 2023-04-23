#include "process.h"
#include "schedule.h"
#include <unistd.h>
#include <elf.h>
#include <stdlib.h>
#include <env.h>
#include <gdt.h>
#include <console.h>
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

static int init_main(void *arg);

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

        process->wait_state = 0;
        process->child = process->older = process->younger = NULL;
    }
    return process;
}

static void add_process(struct ProcessControlBlock *process)
{
    list_add(&process_list, &(process->list_link));
    process->younger = NULL;
    if ((process->older = process->parent->child) != NULL)
    {
        process->older->younger = process;
    }
    process->parent->child = process;
    num_process++;
}

static void remove_process(struct ProcessControlBlock *proc)
{
    list_del(&(proc->list_link));
    if (proc->older != NULL)
    {
        proc->older->younger = proc->younger;
    }
    if (proc->younger != NULL)
    {
        proc->younger->older = proc->older;
    }
    else
    {
        proc->parent->child = proc->older;
    }
    num_process--;
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
    deallocate_pages(virtual_address_to_page((void *) (process->stack)), KERNEL_STACK_PAGE_COUNT);
}

static int setup_page_directory_table(struct VirtualMemory *memory)
{
    struct Page *page;
    if ((page = allocate_pages(1)) == NULL)
    {
        return -E_NO_MEM;
    }

    pde *page_dir = page_to_virtual_address(page);

    // 内核布局对每个进程都是一样的，所以先拷贝内核的页目录表
    memory_copy(page_dir, boot_page_dir, PAGE_SIZE);

    // 页目录表自身映射，线性地址为 VPT
    page_dir[PageDirectoryIndex(VPT)] = PhysicalAddress(page_dir) | PTE_P | PTE_W;
    memory->page_dir = page_dir;

    return 0;
}

static void release_page_directory_table(struct VirtualMemory *memory)
{
    deallocate_pages(virtual_address_to_page(memory->page_dir), 1);
}

static int copy_memory(uint32 clone_flags, struct ProcessControlBlock *process)
{
    struct VirtualMemory *memory, *old_memory = current_process->memory;
    // 若复制的内核进程的内存，则 current_process->memory 为 NULL，因此不会发生拷贝
    if (old_memory == NULL)
        return 0;

    if (clone_flags & CLONE_VM)
    {
        memory = old_memory;
        goto good_memory;
    }

    int ret = -E_NO_MEM;
    if ((memory = create_virtual_memory()) == NULL)
        goto bad_memory;

    if (setup_page_directory_table(memory) != 0)
        goto cleanup_memory;

    ret = duplicate_virtual_memory_mapping(memory, old_memory);
    if (ret != 0)
        goto cleanup_virtual_memory_mapping;


good_memory:
    increase_shared_count(memory);
    process->memory = memory;
    process->cr3 = PhysicalAddress(memory->page_dir);
    return 0;

cleanup_virtual_memory_mapping:
    break_virtual_memory_mapping(memory);
    release_page_directory_table(memory);
cleanup_memory:
    destroy_virtual_memory(memory);
bad_memory:
    return ret;
}

static void fork_ret(void)
{
    fork_rets(current_process->tf);
}

static void hash_process(struct ProcessControlBlock *process)
{
    list_add(hash_list + pid_hash_fn(process->pid), &(process->hash_link));
}

static void unhash_process(struct ProcessControlBlock *process)
{
    list_del(&(process->hash_link));
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

int create_kernel_thread(int (*fn)(void *), void *arg, uint32 clone_flags)
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

    int pid = create_kernel_thread(init_main, "Hello world!!", 0);
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
    int continuous_unused_calling_times = 0;
    while (1)
    {
        continuous_unused_calling_times++;
        if (current_process->need_reschedule)
        {
            continuous_unused_calling_times = 0;
            schedule();
        }

        if (continuous_unused_calling_times > 5)
            panic("idle process run again, times=%d\n", continuous_unused_calling_times);
        kernel_print("idle process run again, times=%d\n", continuous_unused_calling_times);
    }
}

int load_program_code(unsigned char *binary, usize length)
{
    int ret = -E_NO_MEM;

    struct ElfHeader *elf = (struct ElfHeader *) binary;
    struct ProgramHeader *ph = (struct ProgramHeader *) (binary + elf->e_phoff);
    if (elf->e_magic != ELF_MAGIC)
    {
        return -E_INVAL_ELF;
    }

    if (current_process->memory != NULL)
    {
        panic("load program code: current_process->memory must be empty");
    }

    // 申请 VirtualMemory 控制块，维护用户程序内存空间
    struct VirtualMemory *memory;
    if ((memory = create_virtual_memory()) == NULL)
    {
        goto bad_memory;
    }

    // 为用户程序申请页目录表
    if (setup_page_directory_table(memory) != 0)
    {
        goto bad_page_dir_cleanup_memory;
    }

    uint32 vm_flags, perm;
    struct ProgramHeader *ph_end = ph + elf->e_phnum;
    for (; ph < ph_end; ph++)
    {
        // LOAD 段表示必须要加载到内存
        if (ph->p_type != ELF_PT_LOAD || ph->p_offset == 0)
            continue;

        // filesz是 text + rodata + data, memsz是 filesz + bss
        if (ph->p_filesz > ph->p_memsz)
        {
            ret = -E_INVAL_ELF;
            goto bad_cleanup_memory_map;
        }

        if (ph->p_filesz == 0)
            continue;

        vm_flags = 0, perm = PTE_U;
        if (ph->p_flags & ELF_PF_X) vm_flags |= VM_EXEC;
        if (ph->p_flags & ELF_PF_W) vm_flags |= VM_WRITE;
        if (ph->p_flags & ELF_PF_R) vm_flags |= VM_READ;
        if (vm_flags & VM_WRITE) perm |= PTE_W;

        // 为每一个段建立 VirtualMemoryArea，用来指示用户程序哪些部分在内存中还是磁盘上
        if ((ret = build_virtual_memory_mapping(memory, ph->p_vaddr, ph->p_memsz, vm_flags)) != 0)
        {
            goto bad_cleanup_memory_map;
        }

        // from 为当前段在内存中的起始地址
        unsigned char *from = binary + ph->p_offset;
        usize offset, size;

        // start, end 为构建时的线性地址，需要页对齐
        uintptr start = ph->p_vaddr, end = ph->p_vaddr + ph->p_filesz, linear_address = RoundDown(start, PAGE_SIZE);

        struct Page *page;

        // 拷贝代码段.text和数据段.data
        while (start < end)
        {
            // 每次申请一个物理页，逐页拷贝，直到拷完当前段
            if ((page = page_dir_alloc_page(memory->page_dir, linear_address, perm)) == NULL)
            {
                goto bad_cleanup_memory_map;
            }

            offset = start - linear_address, size = PAGE_SIZE - offset, linear_address += PAGE_SIZE;
            if (end < linear_address)
                size -= linear_address - end;

            memory_copy(page_to_virtual_address(page) + offset, from, size);
            start += size, from += size;
        }

        // 紧接着分配静态数据段 .bss
        end = ph->p_vaddr + ph->p_memsz;
        if (start < linear_address)
        {
            /* ph->p_memsz == ph->p_filesz */
            if (start == end)
            {
                continue;
            }
            offset = start + PAGE_SIZE - linear_address, size = PAGE_SIZE - offset;
            if (end < linear_address)
            {
                size -= linear_address - end;
            }
            memory_set(page_to_virtual_address(page) + offset, 0, size);
            start += size;
            assert((end < linear_address && start == end) || (end >= linear_address && start == linear_address));
        }

        while (start < end)
        {
            if ((page = page_dir_alloc_page(memory->page_dir, linear_address, perm)) == NULL)
            {
                goto bad_cleanup_memory_map;
            }
            offset = start - linear_address, size = PAGE_SIZE - offset, linear_address += PAGE_SIZE;
            if (end < linear_address)
            {
                size -= linear_address - end;
            }
            memory_set(page_to_virtual_address(page) + offset, 0, size);
            start += size;
        }
    }

    // 建立用户栈，从 USER_TOP 开始
    vm_flags = VM_READ | VM_WRITE | VM_STACK;
    if ((ret = build_virtual_memory_mapping(memory, USER_STACK_TOP - USER_STACK_SIZE, USER_STACK_SIZE, vm_flags)) != 0)
    {
        goto bad_cleanup_memory_map;
    }
    assert(page_dir_alloc_page(memory->page_dir, USER_STACK_TOP - PAGE_SIZE, PTE_USER) != NULL);
    assert(page_dir_alloc_page(memory->page_dir, USER_STACK_TOP - 2 * PAGE_SIZE, PTE_USER) != NULL);
    assert(page_dir_alloc_page(memory->page_dir, USER_STACK_TOP - 3 * PAGE_SIZE, PTE_USER) != NULL);
    assert(page_dir_alloc_page(memory->page_dir, USER_STACK_TOP - 4 * PAGE_SIZE, PTE_USER) != NULL);

    // 设置当前进程的页目录表，虚拟地址空间，并加载到页目录表物理地址到 crc3 寄存器
    increase_shared_count(memory);
    current_process->memory = memory;
    current_process->cr3 = PhysicalAddress(memory->page_dir);
    lcr3(PhysicalAddress(memory->page_dir));

    struct TrapFrame *tf = current_process->tf;
    memory_set(tf, 0, sizeof(struct TrapFrame));

    tf->tf_cs = USER_CS;
    tf->tf_ds = tf->tf_es = tf->tf_ss = USER_DS;
    tf->tf_esp = USER_STACK_TOP;
    tf->tf_eip = elf->e_entry;
    tf->tf_eflags = FL_IF;

//    print_page_table_item((uintptr) memory->page_dir);

    ret = 0;

exit_func:
    return ret;
bad_cleanup_memory_map:
    break_virtual_memory_mapping(memory);
bad_elf_cleanup_page_dir:
    release_page_directory_table(memory);
bad_page_dir_cleanup_memory:
    destroy_virtual_memory(memory);
bad_memory:
    goto exit_func;
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

    // 为新进程设置内核栈空间
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
        add_process(process);
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
    if (current_process == idle_process)
    {
        panic("idle process exit.\n");
    }

    // 释放进程拥有的内存资源，更新对应的页表项
    struct VirtualMemory *memory = current_process->memory;
    if (memory != NULL)
    {
        lcr3(boot_cr3);
        if (decrease_shared_count(memory) == 0)
        {
            break_virtual_memory_mapping(memory);
            release_page_directory_table(memory);
            destroy_virtual_memory(memory);
        }
        current_process->memory = NULL;
    }

    // 设置进程为僵尸进程，以及进程退出码
    current_process->state = PROCESS_ZOMBIE;
    current_process->exit_code = error_code;

    bool flag;
    struct ProcessControlBlock *process;
    SaveLocalInterrupt(flag);
    {
        // 如果父进程处于等待子进程的状态，那就唤醒父进程，让它完成子进程资源（内核栈，进程控制块）的回收
        process = current_process->parent;
        if (process->wait_state == WT_CHILD)
        {
            wakeup_process(process);
        }

        // 如果当前进程还有子进程，则需要把这些子进程的父进程设置为 init_process，如果有子进程为僵尸进程，则需要唤醒 init_process
        while (current_process->child != NULL)
        {
            process = current_process->child;
            current_process->child = process->child;
            process->younger = NULL;

            if ((process->older = init_process->child) != NULL)
            {
                init_process->child->younger = process;
            }
            process->parent = init_process;
            init_process->child = process;
            if (process->state == PROCESS_ZOMBIE)
            {
                if (init_process->wait_state == WT_CHILD)
                {
                    wakeup_process(init_process);
                }
            }
        }
    }
    RestoreLocalInterrupt(flag);

    schedule();

    panic("do_exit will not return, name=%s, pid=%d\n", current_process->name, current_process->pid);

    return 0;
}

int do_yield(void)
{
    current_process->need_reschedule = 1;
    return 0;
}

int do_execve(const char *name, usize name_length, unsigned char *binary, usize binary_size)
{
    struct VirtualMemory *memory = current_process->memory;
    if (!user_memory_verification(memory, (uintptr) name, name_length, false))
    {
        return -E_INVAL;
    }

    if (name_length > PROCESS_NAME_LEN)
    {
        name_length = PROCESS_NAME_LEN;
    }

    char local_name[PROCESS_NAME_LEN + 1];
    memory_set(local_name, 0, sizeof(local_name));
    memory_copy(local_name, name, name_length);

    if (memory != NULL)
    {
        lcr3(boot_cr3);
        if (decrease_shared_count(memory) == 0)
        {
            break_virtual_memory_mapping(memory);
            release_page_directory_table(memory);
            destroy_virtual_memory(memory);
        }
        current_process->memory = NULL;
    }
    int ret;
    if ((ret = load_program_code(binary, binary_size)) != 0)
    {
        goto execve_exit;
    }
    set_process_name(current_process, local_name);
    return 0;

execve_exit:
    do_exit(ret);
    panic("already exit: %e.\n", ret);
}

int do_wait(int pid, int *code_store)
{
    // current_process 是当前用户进程的父进程
    struct VirtualMemory *memory = current_process->memory;
    if (code_store != NULL)
    {
        if (!user_memory_verification(memory, (uintptr) code_store, sizeof(int), 1))
        {
            return -E_INVAL;
        }
    }

    struct ProcessControlBlock *process;
    bool flag, hask_id;
repeat:
    hask_id = false;

    // 如果 pid 不为0,表示找一个特定的pid,否则就找一个任意的处于退出状态的子进程
    if (pid != 0)
    {
        process = find_process(pid);
        if (process != NULL && process->parent == current_process)
        {
            hask_id = true;
            if (process->state == PROCESS_ZOMBIE)
            {
                goto found;
            }
        }
    }
    else
    {
        process = current_process->child;
        for (; process != NULL; process = process->older)
        {
            hask_id = true;
            if (process->state == PROCESS_ZOMBIE)
            {
                goto found;
            }
        }
    }

    if (hask_id)
    {
        current_process->state = PROCESS_SLEEPING;
        current_process->wait_state = WT_CHILD;
        schedule();
        if (current_process->flags & PF_EXITING)
        {
            do_exit(-E_KILLED);
        }
        goto repeat;
    }
    return -E_BAD_PROC;

found:
    if (process == idle_process || process == init_process)
    {
        panic("wait idle process or init process.\n");
    }

    kernel_print("precess [name=%s, pid=%d] will be released!!!\n", get_process_name(process), process->pid);

    if (code_store != NULL)
    {
        *code_store = process->exit_code;
    }

    SaveLocalInterrupt(flag);
    {
        unhash_process(process);
        remove_process(process);
    }
    RestoreLocalInterrupt(flag);

    release_kernel_stack(process);
    kernel_free(process, sizeof(struct ProcessControlBlock));
    return 0;
}

int do_kill(int pid)
{
    return 0;
}

static int kernel_execve(const char *name, char *binary, usize size)
{
    int ret, len = string_length(name);
    asm volatile (
            "int %1;"
            : "=a" (ret)
            : "i" (T_SYSCALL), "0" (SYS_EXEC), "d" (name), "c" (len), "b" (binary), "D" (size)
            : "memory");
    return ret;
}

// user_main 是每一个用户程序的入口，主要为加载用户程序，申请内存空间等
static int user_main(void *arg)
{
    kernel_execve("hello-program", temp_user_space_start, PAGE_SIZE);
    panic("user_main execve failed.\n");
}

static int init_main(void *arg)
{
    kernel_print("this init process, pid = %d, name = \"%s\"\n", current_process->pid, get_process_name(current_process));

    int pid = create_kernel_thread(user_main, arg, 0);
    if (pid <= 0)
    {
        panic("create user_main failed.\n");
    }

    while (do_wait(0, NULL) == 0)
    {
        schedule();
    }

    return 0;
}
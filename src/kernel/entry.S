#include <bitzone.h>
#include <page.h>

#define REALLOC(x) (x - KERNEL_BASE)

.text
.globl kernel_entry
kernel_entry:
    # 将内核页目录表的物理地址加载到CR3
    # 内核页目录表在线性地址3G~4G的区域内，需要映射在物理地址0～1G的区域内
    movl $REALLOC(__boot_pgdir), %eax
    movl %eax, %cr3

    # 开启页功能
    movl %cr0, %eax
    orl $(CR0_PE | CR0_PG | CR0_AM | CR0_WP | CR0_NE | CR0_TS | CR0_EM | CR0_MP), %eax
    andl $~(CR0_TS | CR0_EM), %eax
    movl %eax, %cr0

    # 此时页功能开启，eip的值为0x100000+,指向物理地址
    # 但执行下一条指令会将eip中的地址作为线性地址解析
    # 因此，需要临时建立虚拟地址 0 ~ 4M 到物理地址 0 ~ 4M 的映射，即页目录表第0项
    leal next, %eax
    jmp *%eax

next:
    # 将页目录表第0项清零，即取消线性地址0~4MB对物理地址0~4MB的一一映射
    xorl %eax, %eax
    movl %eax, __boot_pgdir

    movl $0x0, %ebp
    # the kernel stack region is from boot_stack -- boot_stack_top,
    # the kernel stack size is KERNEL_STACK_SIZE (8KB)defined in memlayout.h
    movl $boot_stack_top, %esp
    # now kernel stack is ready , call the first C function
    call kernel_init

spin:
    jmp spin

# 定义内核栈，大小为2页，8192 字节
.data
.align PAGE_SIZE
    .globl boot_stack
boot_stack:
    .space KERNEL_STACK_SIZE
    .globl boot_stack_top
boot_stack_top:

# 定义一张页目录表，用来表示内核空间
# 因为内核很小，小于4MB，所以只用一页即可；那么页目录表中也只需一个页目录项
.section .data.pgdir
.align PAGE_SIZE
__boot_pgdir:
.globl __boot_pgdir
    # 将 虚拟地址 0 ~ 4M 映射到物理地址 0 ~ 4M (temporary)
    # 0号 页目录表项，4字节，指向__boot_pt1页表
    .long REALLOC(__boot_pt1) + (PTE_P | PTE_U | PTE_W)

    # 补齐字节数到3072
    .space (KERNEL_BASE >> PGSHIFT >> 10 << 2) - (. - __boot_pgdir) # pad to PDE of KERNEL_BASE

    # 将 虚拟地址 3G + (0 ~ 4M) 映射到物理地址 0 ~ 4M
    # 728号页目录表项，4字节，指向__boot_pt1页表
    .long REALLOC(__boot_pt1) + (PTE_P | PTE_U | PTE_W)

    # 补齐一页大小，4096字节
    .space PAGE_SIZE - (. - __boot_pgdir) # pad to PAGE_SIZE

# 定义一张页表。一共1024个页表项，页表项中的地址分别是0,4096,8192,...
# 1024页，可表示4MB大小;并且是用来表示物理地址0～4MB的空间
.set i, 0
__boot_pt1:
.rept 1024
    .long i * PAGE_SIZE + (PTE_P | PTE_W)
    .set i, i + 1
.endr

#include "asm.h"

# Start the CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

.set PROT_MODE_CSEG,        0x8                     # 代码段记录的起始地址
.set PROT_MODE_DSEG,        0x10                    # 内核数据段记录的起始地址
.set CR0_PE_ON,             0x1                     # CR0寄存器保护模式位开启标识
.set SMAP,                  0x534d4150

# start address should be 0:7c00, in real mode, the beginning address of the running bootloader
.globl start
start:
.code16
    cli                                             # 设置EFLAGS寄存器中断标识位，关闭中断
    cld                                             # 设置EFLAGS寄存器方向标识位，设置esp，edi寄存器为递增

    xorw %ax, %ax                                   # 异或结果为0
    movw %ax, %ds                                   # ds寄存器置为0
    movw %ax, %es                                   # es寄存器置为0
    movw %ax, %ss                                   # ss寄存器置为0

    # 开启 A20:
    # A20地址线默认为0,即关闭;当访问超过1M的内存地址时，会发生回卷现象，即从0号地址重新循环
    # 寻址;所以要开启A20，这样就能访问超出1M地址的内存空间。

# 8042是键盘控制器，有输入端口P1，输出端口P2
seta20.1:
    # 等待8042输入缓冲区为空
    # 从0x64端口读取一个字节的数据到al
    # 测试该字节的第1位是否为0,是1就继续检查
    inb $0x64, %al                                  
    testb $0x2, %al
    jnz seta20.1

    # 向0x64端口发送数据0xd1（向键盘控制器8042 P2端口 发送数据）
    movb $0xd1, %al
    outb %al, $0x64

seta20.2:
    # 等待8042输入缓冲区为空
    inb $0x64, %al
    testb $0x2, %al
    jnz seta20.2

    # 向0x60端口发送数据0xdf（0xdf = 11011111，意味着将P2的A20位设置为1）
    movb $0xdf, %al
    outb %al, $0x60

probe_memory:
    # 使用BIOS中断，探测物理内存，探测结果在物理地址 0x8000，后续由 E820Map 结构体来解析使用
    movl $0, 0x8000
    xorl %ebx, %ebx
    movw $0x8004, %di
start_probe:
    movl $0xE820, %eax
    movl $20, %ecx
    movl $SMAP, %edx
    int $0x15
    jnc cont
    movw $12345, 0x8000
    jmp finish_probe
cont:
    addw $20, %di
    incl 0x8000
    cmpl $0, %ebx
    jnz start_probe
finish_probe:
    # 开启保护模式。先将GDT的地址加载到GDTR寄存器中;将CR0寄存器中的PE位设置为1,即开启保护模式
    lgdt gdtdesc
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax
    movl %eax, %cr0

    # Jump to next instruction, but in 32-bit code segment.
    # Switches processor into 32-bit mode.
    ljmp $PROT_MODE_CSEG, $protcseg

.code32
protcseg:
    # 将以下寄存器的值都设置为GDT中数据段记录的位置
    movw $PROT_MODE_DSEG, %ax                       # Our data segment selector
    movw %ax, %ds                                   # -> DS: Data Segment
    movw %ax, %es                                   # -> ES: Extra Segment
    movw %ax, %fs                                   # -> FS
    movw %ax, %gs                                   # -> GS
    movw %ax, %ss                                   # -> SS: Stack Segment

    # 设置堆栈寄存器，因为BIOS会从0x7c00地址开始运行，可以将这之前的地址设置为栈，入栈地址减小;
    # 之后调用C程序中的bootmain函数
    movl $0x0, %ebp
    movl $start, %esp
    call boot_main

    # 如果bootmain返回了（理应不会），就在这一直循环
spin:
    jmp spin

# 定义GDT表
.p2align 2                                          # 强制4字节内存对齐
gdt:
    SEG_NULLASM                                     # 空GDT记录
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # 代码段记录
    SEG_ASM(STA_W, 0x0, 0xffffffff)                 # 数据段记录

gdtdesc:
    .word 0x17                                      # sizeof(gdt) - 1
    .long gdt                                       # GDT段的地址

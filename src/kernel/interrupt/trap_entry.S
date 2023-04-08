#include <gdt.h>

# 所有的中断都会经过这里，再调到C代码trap，由C程序来分发处理函数
.text
.globl __all_traps
__all_traps:
    # 将各个寄存器压栈，在栈内存构造出与C程序 struct TrapFrame 相同的内存结构
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    pushal

    # load GD_KDATA into %ds and %es to set up data segments for kernel
    movl $GD_KDATA, %eax
    movw %ax, %ds
    movw %ax, %es

    # esp所指的地址就是行参的地址，行参本身也占4字节，需要压入栈
    pushl %esp

    # 调用函数trap(tf), 将行参地址传给它
    call trap

    # 将行参出栈
    popl %esp

.globl __trap_ret
__trap_ret:
    # 依次出栈，还原各寄存器的值
    popal
    popl %gs
    popl %fs
    popl %es
    popl %ds

    # 将中断向量号 和 错误码 出栈
    addl $0x8, %esp
    iret

.globl fork_rets
fork_rets:
    # 把 %esp 设置为新进程(to) TrapFrame 首地址
    movl 4(%esp), %esp
    jmp __trap_ret

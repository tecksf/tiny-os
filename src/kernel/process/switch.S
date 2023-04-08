# 进行进程间的上下文切换从from进程切换到to进程
.text
.globl switch_to
switch_to:

    # 当前栈顶元素依次是 返回地址，from上下文首地址，to上下文首地址
    # 存储from用到的寄存器
    movl 4(%esp), %eax          # 将from上下文的首地址存储到eax
    popl 0(%eax)                # 将返回地址存储到from的eip，此时栈顶是from上下文首地址
    movl %esp, 4(%eax)
    movl %ebx, 8(%eax)
    movl %ecx, 12(%eax)
    movl %edx, 16(%eax)
    movl %esi, 20(%eax)
    movl %edi, 24(%eax)
    movl %ebp, 28(%eax)

    # 还原to用到的寄存器
    movl 4(%esp), %eax          # 将to上下文的首地址存储到eax
    movl 28(%eax), %ebp
    movl 24(%eax), %edi
    movl 20(%eax), %esi
    movl 16(%eax), %edx
    movl 12(%eax), %ecx
    movl 8(%eax), %ebx
    movl 4(%eax), %esp

    pushl 0(%eax)               # 将to中的eip入栈，ret会跳转到eip对应的指令

    ret

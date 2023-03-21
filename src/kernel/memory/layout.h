#ifndef __MEMORY_LAYOUT_H__
#define __MEMORY_LAYOUT_H__

#define KERNEL_BASE                     0xC0000000              // 内核线性地址基址 3G
#define KERNEL_MEMORY_SIZE              0x38000000              // 内核大小
#define KERNEL_TOP                      (KERNEL_BASE + KERNEL_MEMORY_SIZE)

#define VPT                             0xFAC00000

#define KERNEL_STACK_PAGE_COUNT         2
#define KERNEL_STACK_SIZE               (KERNEL_STACK_PAGE_COUNT * PAGE_SIZE)       // 内核栈大小，2页


#endif // __MEMORY_LAYOUT_H__

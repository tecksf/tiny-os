#ifndef __MEMORY_LAYOUT_H__
#define __MEMORY_LAYOUT_H__

#define PAGE_SIZE                       4096
#define KERNEL_BASE                     0xC0000000              // 内核线性地址基址 3G
#define KERNEL_MEMORY_SIZE              0x38000000              // 内核大小
#define KERNEL_TOP                      (KERNEL_BASE + KERNEL_MEMORY_SIZE)

#define VPT                             0xFAC00000

#define USER_TOP                        0xB0000000
#define USER_STACK_TOP                  USER_TOP
#define USER_STACK_PAGE                 256                                 // # of pages in user stack
#define USER_STACK_SIZE                 (USER_STACK_PAGE * PAGE_SIZE)       // sizeof user stack

#define USER_BASE                       0x00200000
#define USER_TEXT                       0x00800000                      // where user programs generally begin
#define USER_STAB                       USER_BASE                       // the location of the user STABS data structure

#define KERNEL_STACK_PAGE_COUNT         2
#define KERNEL_STACK_SIZE               (KERNEL_STACK_PAGE_COUNT * PAGE_SIZE)       // 内核栈大小，2页

#define UserAccess(start, end)          (USER_BASE <= (start) && (start) < (end) && (end) <= USER_TOP)
#define KernelAccess(start, end)        (KERNEL_BASE <= (start) && (start) < (end) && (end) <= KERNEL_TOP)

#endif // __MEMORY_LAYOUT_H__

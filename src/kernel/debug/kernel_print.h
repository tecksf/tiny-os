#ifndef __KERNEL_DEBUG_KERNEL_PRINT_H__
#define __KERNEL_DEBUG_KERNEL_PRINT_H__

#include <defs.h>

void print_kernel_info();
void print_stack_frame();
void print_debug_info(uintptr eip);

#endif // __KERNEL_DEBUG_KERNEL_PRINT_H__

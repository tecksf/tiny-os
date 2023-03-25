#ifndef __CORE_ENV_H__
#define __CORE_ENV_H__

#include <defs.h>

extern usize max_swap_offset;

extern volatile int swap_init_ok;

extern volatile unsigned int page_fault_num;

extern struct VirtualMemory *virtual_memory_verification;


#endif // __CORE_ENV_H__

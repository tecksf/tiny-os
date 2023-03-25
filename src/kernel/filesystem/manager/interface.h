#ifndef __FILESYSTEM_MANAGER_INTERFACE_H__
#define __FILESYSTEM_MANAGER_INTERFACE_H__

#include <defs.h>
#include "virtual.h"
#include <page.h>

struct SwapManager
{
    const char *name;
    /* Global initialization for the swap manager */
    int (*init)(void);
    /* Initialize the priv data inside mm_struct */
    int (*init_virtual_memory)(struct VirtualMemory *memory);
    /* Called when tick interrupt occured */
    int (*tick_event)(struct VirtualMemory *memory);
    /* Called when map a swappable page into the mm_struct */
    int (*map_swappable)(struct VirtualMemory *memory, uintptr address, struct Page *page, int swap_in);
    /* When a page is marked as shared, this routine is called to
      * delete the addr entry from the swap manager */
    int (*set_unswappable)(struct VirtualMemory *memory, uintptr address);
    /* Try to swap out a page, return then victim */
    int (*swap_out_victim)(struct VirtualMemory *memory, struct Page **ptr_page, int in_tick);
    /* check the page relpacement algorithm */
    int (*check_swap)(void);
};

#endif // __FILESYSTEM_MANAGER_INTERFACE_H__

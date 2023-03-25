#ifndef __FILESYSTEM_SWAP_H__
#define __FILESYSTEM_SWAP_H__

#include <defs.h>
#include <page.h>
#include <assert.h>
#include "virtual.h"

/* *
 * SwapEntry
 * --------------------------------------------
 * |         offset        |   reserved   | 0 |
 * --------------------------------------------
 *           24 bits            7 bits    1 bit
 * */
typedef pte SwapEntry; // 页目录项也可以是swap项

#define SwapOffset(entry) ({                            \
    usize __offset = (entry >> 8);                      \
    if (!(__offset > 0 && __offset < max_swap_offset))  \
    {                                                   \
        panic("invalid swap_entry_t = %08x.\n", entry); \
    }                                                   \
    __offset;                                           \
})
#define MAX_SWAP_OFFSET_LIMIT (1 << 24)


void swap_fs_init(void);
int swap_fs_read(SwapEntry entry, struct Page *page);
int swap_fs_write(SwapEntry entry, struct Page *page);


int swap_init(void);
int swap_init_virtual_memory(struct VirtualMemory *memory);
int swap_tick_event(struct VirtualMemory *memory);
int swap_map_swappable(struct VirtualMemory *memory, uintptr address, struct Page *page, int swap_in);
int swap_set_unswappable(struct VirtualMemory *memory, uintptr address);
int swap_out(struct VirtualMemory *memory, int n, int in_tick);
int swap_in(struct VirtualMemory *memory, uintptr address, struct Page **ptr_result);

#endif // __FILESYSTEM_SWAP_H__

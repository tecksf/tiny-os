#include "swap.h"
#include "fs.h"
#include <ide.h>
#include <physical.h>
#include <console.h>
#include "manager/fifo.h"

static struct SwapManager *manager;
usize max_swap_offset;
volatile int swap_init_ok = 0;

static void check_swap(void);

void swap_fs_init(void)
{
    if (!ide_device_valid(SWAP_DEV_NO))
    {
        panic("swap fs isn't available.\n");
    }
    max_swap_offset = ide_device_size(SWAP_DEV_NO) / (PAGE_SIZE / SECTION_SIZE);
}

int swap_fs_read(SwapEntry entry, struct Page *page)
{
    return ide_read_sections(SWAP_DEV_NO, SwapOffset(entry) * SECTIONS_PER_PAGE, page_to_virtual_address(page), SECTIONS_PER_PAGE);
}

int swap_fs_write(SwapEntry entry, struct Page *page)
{
    return ide_write_sections(SWAP_DEV_NO, SwapOffset(entry) * SECTIONS_PER_PAGE, page_to_virtual_address(page), SECTIONS_PER_PAGE);
}


int swap_init(void)
{
    swap_fs_init();

    if (!(1024 <= max_swap_offset && max_swap_offset < MAX_SWAP_OFFSET_LIMIT))
    {
        panic("bad max_swap_offset %08x.\n", max_swap_offset);
    }

    manager = &swap_manager_fifo;
    int r = manager->init();

    if (r == 0)
    {
        swap_init_ok = 1;
        kernel_print("SWAP: manager = %s\n", manager->name);
        check_swap();
    }

    return r;
}

int swap_init_virtual_memory(struct VirtualMemory *memory)
{
    return manager->init_virtual_memory(memory);
}

int swap_tick_event(struct VirtualMemory *mm)
{
    return manager->tick_event(mm);
}

int swap_map_swappable(struct VirtualMemory *memory, uintptr address, struct Page *page, int swap_in)
{
    return manager->map_swappable(memory, address, page, swap_in);
}

int swap_set_unswappable(struct VirtualMemory *memory, uintptr address)
{
    return manager->set_unswappable(memory, address);
}

int swap_out(struct VirtualMemory *memory, int n, int in_tick)
{
    int i;
    for (i = 0; i != n; ++i)
    {
        uintptr v;
        //struct Page **ptr_page=NULL;
        struct Page *page;
        // kernel_print("i %d, SWAP: call swap_out_victim\n",i);
        int r = manager->swap_out_victim(memory, &page, in_tick);
        if (r != 0)
        {
            kernel_print("i %d, swap_out: call swap_out_victim failed\n", i);
            break;
        }
        //assert(!PageReserved(page));

        //kernel_print("SWAP: choose victim page 0x%08x\n", page);

        v = page->pra_vaddr;
        pte *entry = get_page_table_entry(memory->page_dir, v, 0);
        assert((*entry & PTE_P) != 0);

        if (swap_fs_write((page->pra_vaddr / PAGE_SIZE + 1) << 8, page) != 0)
        {
            kernel_print("SWAP: failed to save\n");
            manager->map_swappable(memory, v, page, 0);
            continue;
        }
        else
        {
            kernel_print("swap_out: i %d, store page in vaddr 0x%x to disk swap entry %d\n", i, v, page->pra_vaddr / PAGE_SIZE + 1);
            *entry = (page->pra_vaddr / PAGE_SIZE + 1) << 8;
            deallocate_pages(page, 1);
        }

        tlb_invalidate(memory->page_dir, v);
    }
    return i;
}

int swap_in(struct VirtualMemory *memory, uintptr address, struct Page **ptr_result)
{
    struct Page *result = allocate_pages(1);
    assert(result != NULL);

    pte *entry = get_page_table_entry(memory->page_dir, address, 0);
    // kernel_print("SWAP: load entry %x swap entry %d to vaddr 0x%08x, page %x, No %d\n", entry, (*entry)>>8, addr, result, (result-pages));

    int r;
    if ((r = swap_fs_read((*entry), result)) != 0)
    {
        assert(r != 0);
    }
    kernel_print("swap_in: load disk swap entry %d with swap_page in virtual address 0x%x\n", (*entry) >> 8, address);
    *ptr_result = result;
    return 0;
}

static void check_swap(void)
{

}

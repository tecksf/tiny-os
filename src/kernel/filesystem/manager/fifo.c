#include "fifo.h"
#include <env.h>
#include <list.h>
#include <assert.h>
#include <console.h>
#include <page.h>

ListEntry pra_list_head;

/*
 * (2) _fifo_init_mm: init pra_list_head and let  mm->sm_priv point to the addr of pra_list_head.
 *              Now, From the memory control struct VirtualMemory, we can access FIFO PRA
 */
static int _fifo_init_virtual_memory(struct VirtualMemory *memory)
{
    list_init(&pra_list_head);
    memory->swap_manager_private_data = &pra_list_head;
//    kernel_print(" memory->swap_manager_private_data %x in fifo_init_mm\n",memory->swap_manager_private_data);
    return 0;
}

/*
 * (3)_fifo_map_swappable: According FIFO PRA, we should link the most recent arrival page at the back of pra_list_head qeueue
 */
static int _fifo_map_swappable(struct VirtualMemory *memory, uintptr address, struct Page *page, int swap_in)
{
    ListEntry *head = (ListEntry *) memory->swap_manager_private_data;
    ListEntry *entry = &(page->pra_page_link);

    assert(entry != NULL && head != NULL);
    //record the page access situlation
    /*LAB3 EXERCISE 2: YOUR CODE*/
    //(1)link the most recent arrival page at the back of the pra_list_head qeueue.
    list_add(head, entry);
    return 0;
}

/*
 *  (4)_fifo_swap_out_victim: According FIFO PRA, we should unlink the  earliest arrival page in front of pra_list_head qeueue,
 *                            then assign the value of *ptr_page to the addr of this page.
 */
static int _fifo_swap_out_victim(struct VirtualMemory *mm, struct Page **ptr_page, int in_tick)
{
    ListEntry *head = (ListEntry *) mm->swap_manager_private_data;
    assert(head != NULL);
    assert(in_tick == 0);
    /* Select the victim */
    /*LAB3 EXERCISE 2: YOUR CODE*/
    //(1)  unlink the  earliest arrival page in front of pra_list_head qeueue
    //(2)  assign the value of *ptr_page to the addr of this page
    /* Select the tail */
    ListEntry *le = head->prev;
    assert(head != le);
    struct Page *p = OffsetOfPage(le, pra_page_link);
    list_del(le);
    assert(p != NULL);
    *ptr_page = p;
    return 0;
}

static int _fifo_check_swap(void)
{
    kernel_print("write Virt Page c in fifo_check_swap\n");
    *(unsigned char *) 0x3000 = 0x0c;
    assert(page_fault_num == 4);
    kernel_print("write Virt Page a in fifo_check_swap\n");
    *(unsigned char *) 0x1000 = 0x0a;
    assert(page_fault_num == 4);
    kernel_print("write Virt Page d in fifo_check_swap\n");
    *(unsigned char *) 0x4000 = 0x0d;
    assert(page_fault_num == 4);
    kernel_print("write Virt Page b in fifo_check_swap\n");
    *(unsigned char *) 0x2000 = 0x0b;
    assert(page_fault_num == 4);
    kernel_print("write Virt Page e in fifo_check_swap\n");
    *(unsigned char *) 0x5000 = 0x0e;
    assert(page_fault_num == 5);
    kernel_print("write Virt Page b in fifo_check_swap\n");
    *(unsigned char *) 0x2000 = 0x0b;
    assert(page_fault_num == 5);
    kernel_print("write Virt Page a in fifo_check_swap\n");
    *(unsigned char *) 0x1000 = 0x0a;
    assert(page_fault_num == 6);
    kernel_print("write Virt Page b in fifo_check_swap\n");
    *(unsigned char *) 0x2000 = 0x0b;
    assert(page_fault_num == 7);
    kernel_print("write Virt Page c in fifo_check_swap\n");
    *(unsigned char *) 0x3000 = 0x0c;
    assert(page_fault_num == 8);
    kernel_print("write Virt Page d in fifo_check_swap\n");
    *(unsigned char *) 0x4000 = 0x0d;
    assert(page_fault_num == 9);
    kernel_print("write Virt Page e in fifo_check_swap\n");
    *(unsigned char *) 0x5000 = 0x0e;
    assert(page_fault_num == 10);
    kernel_print("write Virt Page a in fifo_check_swap\n");
    assert(*(unsigned char *) 0x1000 == 0x0a);
    *(unsigned char *) 0x1000 = 0x0a;
    assert(page_fault_num == 11);

    return 0;
}

static int _fifo_init(void)
{
    return 0;
}

static int _fifo_set_unswappable(struct VirtualMemory *mm, uintptr address)
{
    return 0;
}

static int _fifo_tick_event(struct VirtualMemory *memory)
{
    return 0;
}

struct SwapManager swap_manager_fifo = {
        .name = "fifo swap manager",
        .init = &_fifo_init,
        .init_virtual_memory = &_fifo_init_virtual_memory,
        .tick_event = &_fifo_tick_event,
        .map_swappable = &_fifo_map_swappable,
        .set_unswappable = &_fifo_set_unswappable,
        .swap_out_victim = &_fifo_swap_out_victim,
        .check_swap = &_fifo_check_swap,
};
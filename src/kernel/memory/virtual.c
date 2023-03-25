#include <env.h>
#include "virtual.h"
#include "physical.h"
#include "error.h"
#include "swap.h"
#include "stdio.h"

struct VirtualMemory *virtual_memory_verification;
volatile unsigned int page_fault_num = 0;

static inline void check_virtual_memory_address_overlap(struct VirtualMemoryAddress *prev, struct VirtualMemoryAddress *next)
{
    assert(prev->start < prev->end);
    assert(prev->end <= next->start);
    assert(next->start < next->end);
}

static void check_virtual_memory_address(void)
{
    printf("check_virtual_memory_address() succeeded!\n");
}

static void check_page_fault(void)
{
    printf("check_page_fault() succeeded!\n");
}

static void check_virtual_memory(void)
{
    check_virtual_memory_address();
    check_page_fault();
    printf("check_virtual_memory() succeeded.\n");
}

void virtual_memory_init(void)
{
    check_virtual_memory();
}

int do_page_fault(struct VirtualMemory *memory, uint32 error_code, uintptr address)
{
    int rc = -E_INVAL;

    page_fault_num++;

    // 先确认是否为合法的虚拟页地址
    struct VirtualMemoryAddress *vma = find_virtual_memory_address(memory, address);
    if (vma == NULL || vma->start > address)
    {
        printf("not valid address %x, and can not find it in vma\n", address);
        goto failed;
    }

    switch (error_code & 3)
    {
        default:
            /* error code flag : default is 3 ( W/R=1, P=1): write, present */
        case 2: /* error code flag : (W/R=1, P=0): write, not present */
            if (!(vma->flags & VM_WRITE))
            {
                printf("do_page_fault failed: error code flag = write AND not present, but the addr's vma cannot write\n");
                goto failed;
            }
            break;
        case 1: /* error code flag : (W/R=0, P=1): read, present */
            printf("do_page_fault failed: error code flag = read AND present\n");
            goto failed;
        case 0: /* error code flag : (W/R=0, P=0): read, not present */
            if (!(vma->flags & (VM_READ | VM_EXEC)))
            {
                printf("do_page_fault failed: error code flag = read AND not present, but the addr's vma cannot read or exec\n");
                goto failed;
            }
    }

    uint32 perm = PTE_U;
    if (vma->flags & VM_WRITE)
    {
        perm |= PTE_W;
    }
    address = RoundDown(address, PAGE_SIZE);

    rc = -E_NO_MEM;

    pte *entry = NULL;
    if ((entry = get_page_table_entry(memory->page_dir, address, 1)) == NULL)
    {
        printf("get_pte in do_page_fault failed\n");
        goto failed;
    }

    if (*entry == 0)
    {
        // 页表项为0,则物理页不存在, 分配一个新页，并映射到物理地址和逻辑地址
        if (page_dir_alloc_page(memory->page_dir, address, perm) == NULL)
        {
            printf("page_dir_alloc_page in do_page_fault failed\n");
            goto failed;
        }
    }
    else
    {
        // PTE是swap项时, 需要从磁盘读入被换出的页
        // and call page_insert to map the phy addr with logical addr
        if (swap_init_ok)
        {
            struct Page *page = NULL;
            if ((rc = swap_in(memory, address, &page)) != 0)
            {
                printf("swap_in in do_page_fault failed\n");
                goto failed;
            }
            page_insert(memory->page_dir, page, address, perm);
            swap_map_swappable(memory, address, page, 1);
            page->pra_vaddr = address;
        }
        else
        {
            printf("no swap_init_ok but entry is %x, failed\n", *entry);
            goto failed;
        }
    }
    rc = 0;

failed:
    return rc;
}

struct VirtualMemory *create_virtual_memory(void)
{
    struct VirtualMemory *memory = kernel_malloc(sizeof(struct VirtualMemory));

    if (memory != NULL)
    {
        list_init(&(memory->mmap_list));
        memory->mmap_cache = NULL;
        memory->page_dir = NULL;
        memory->map_count = 0;

        if (swap_init_ok)
            swap_init_virtual_memory(memory);
        else
            memory->swap_manager_private_data = NULL;
    }
    return memory;
}

void destroy_virtual_memory(struct VirtualMemory *memory)
{
    ListEntry *list = &(memory->mmap_list), *le;
    while ((le = list_next(list)) != list)
    {
        list_del(le);
        kernel_free(OffsetOfVirtualMemoryAddress(le, list_link), sizeof(struct VirtualMemoryAddress));
    }
    kernel_free(memory, sizeof(struct VirtualMemory));
}


struct VirtualMemoryAddress *create_virtual_memory_address(uintptr start, uintptr end, uint32 flags)
{
    struct VirtualMemoryAddress *address = kernel_malloc(sizeof(struct VirtualMemoryAddress));

    if (address != NULL)
    {
        address->start = start;
        address->end = end;
        address->flags = flags;
    }
    return address;
}

struct VirtualMemoryAddress *find_virtual_memory_address(struct VirtualMemory *memory, uintptr address)
{
    struct VirtualMemoryAddress *vma = NULL;
    if (memory != NULL)
    {
        vma = memory->mmap_cache;
        if (!(vma != NULL && vma->start <= address && vma->end > address))
        {
            bool found = 0;
            ListEntry *list = &(memory->mmap_list), *le = list;
            while ((le = list_next(le)) != list)
            {
                vma = OffsetOfVirtualMemoryAddress(le, list_link);
                if (vma->start <= address && address < vma->end)
                {
                    found = 1;
                    break;
                }
            }

            if (!found)
                vma = NULL;
        }

        if (vma != NULL)
        {
            memory->mmap_cache = vma;
        }
    }
    return vma;
}

void insert_virtual_memory_address(struct VirtualMemory *memory, struct VirtualMemoryAddress *address)
{
    assert(address->start < address->end);
    ListEntry *list = &(memory->mmap_list);
    ListEntry *le_prev = list, *le_next;

    ListEntry *le = list;
    while ((le = list_next(le)) != list)
    {
        struct VirtualMemoryAddress *mmap_prev = OffsetOfVirtualMemoryAddress(le, list_link);
        if (mmap_prev->start > address->start)
            break;

        le_prev = le;
    }

    le_next = list_next(le_prev);

    /* 检查虚拟内存段是否有重叠 */
    if (le_prev != list)
        check_virtual_memory_address_overlap(OffsetOfVirtualMemoryAddress(le_prev, list_link), address);
    if (le_next != list)
        check_virtual_memory_address_overlap(address, OffsetOfVirtualMemoryAddress(le_next, list_link));

    address->virtual_memory = memory;
    list_add_after(le_prev, &(address->list_link));

    memory->map_count++;
}
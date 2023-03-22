#include "first_fit.h"

// 管理所有空闲的页
struct FreeArea free_area;

static void init(void)
{
    list_init(&free_area.free_list);
    free_area.num = 0;
}

// 初始化可用页的Page结构，并将可用页以链表形式维护在free_area下
static void init_memory_map(struct Page *base, usize n)
{
    struct Page *p = base;
    for (; p != base + n; p++)
    {
        p->flags = p->property = 0;
        set_page_reference(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    free_area.num += n;
    list_add_before(&free_area.free_list, &(base->page_link));
}

// 首次适应算法
static struct Page *alloc_pages(usize n)
{
    if (n > free_area.num)
        return NULL;

    struct Page *page = NULL;
    ListEntry *le = &free_area.free_list;

    while ((le = list_next(le)) != &free_area.free_list)
    {
        struct Page *p;
        p = OffsetOfPage(le, page_link);
        if (p->property >= n)
        {
            page = p;
            break;
        }
    }

    if (page != NULL)
    {
        if (page->property > n)
        {
            struct Page *p = page + n;
            p->property = page->property - n;
            SetPageProperty(p);
            list_add_after(&(page->page_link), &(p->page_link));
        }

        list_del(&(page->page_link));
        free_area.num -= n;
        ClearPageProperty(page);
    }

    return page;
}

static void free_pages(struct Page *base, usize n)
{
    struct Page *p = base;
    for (; p != base + n; p++)
    {
        p->flags = 0;
        set_page_reference(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    ListEntry *le = list_next(&free_area.free_list);
    while (le != &free_area.free_list)
    {
        p = OffsetOfPage(le, page_link);
        le = list_next(le);
        if (base + base->property == p)
        {
            base->property += p->property;
            ClearPageProperty(p);
            list_del(&(p->page_link));
        }
        else if (p + p->property == base)
        {
            p->property += base->property;
            ClearPageProperty(base);
            base = p;
            list_del(&(p->page_link));
        }
    }
    free_area.num += n;
    le = list_next(&free_area.free_list);
    while (le != &free_area.free_list)
    {
        p = OffsetOfPage(le, page_link);
        if (base + base->property <= p)
        {
            break;
        }
        le = list_next(le);
    }
    list_add_before(le, &(base->page_link));
}

static usize get_number_of_free_pages(void)
{
    return free_area.num;
}

static void check(void)
{
}

const struct PhysicalMemoryManager first_fit_memory_manager = {
        .name = "first_fit_memory_manager",
        .init = init,
        .init_memory_map = init_memory_map,
        .alloc_pages = alloc_pages,
        .free_pages = free_pages,
        .get_number_of_free_pages = get_number_of_free_pages,
        .check = check,
};

#ifndef __BASE_LIST_H__
#define __BASE_LIST_H__

#include "defs.h"

typedef struct ListEntry
{
    struct ListEntry *prev, *next;
} ListEntry;


static inline void list_init(ListEntry *elm) __attribute__((always_inline));
static inline void list_add(ListEntry *list_elm, ListEntry *elm) __attribute__((always_inline));
static inline void list_add_before(ListEntry *list_elm, ListEntry *elm) __attribute__((always_inline));
static inline void list_add_after(ListEntry *list_elm, ListEntry *elm) __attribute__((always_inline));
static inline void list_del(ListEntry *list_elm) __attribute__((always_inline));
static inline void list_del_init(ListEntry *list_elm) __attribute__((always_inline));
static inline bool list_empty(ListEntry *list) __attribute__((always_inline));
static inline ListEntry *list_next(ListEntry *list_elm) __attribute__((always_inline));
static inline ListEntry *list_prev(ListEntry *list_elm) __attribute__((always_inline));

static inline void __list_add(ListEntry *elm, ListEntry *prev, ListEntry *next) __attribute__((always_inline));
static inline void __list_del(ListEntry *prev, ListEntry *next) __attribute__((always_inline));

static inline void list_init(ListEntry *elm)
{
    elm->prev = elm->next = elm;
}

static inline void list_add(ListEntry *list_elm, ListEntry *elm)
{
    list_add_after(list_elm, elm);
}

static inline void list_add_before(ListEntry *list_elm, ListEntry *elm)
{
    __list_add(elm, list_elm->prev, list_elm);
}

static inline void list_add_after(ListEntry *list_elm, ListEntry *elm)
{
    __list_add(elm, list_elm, list_elm->next);
}

static inline void list_del(ListEntry *list_elm)
{
    __list_del(list_elm->prev, list_elm->next);
}

static inline void list_del_init(ListEntry *list_elm)
{
    list_del(list_elm);
    list_init(list_elm);
}

static inline bool list_empty(ListEntry *list)
{
    return list->next == list;
}

static inline ListEntry *list_next(ListEntry *list_elm)
{
    return list_elm->next;
}

static inline ListEntry *list_prev(ListEntry *list_elm)
{
    return list_elm->prev;
}

static inline void __list_add(ListEntry *elm, ListEntry *prev, ListEntry *next)
{
    prev->next = next->prev = elm;
    elm->next = next;
    elm->prev = prev;
}

static inline void __list_del(ListEntry *prev, ListEntry *next)
{
    prev->next = next;
    next->prev = prev;
}

#endif // __BASE_LIST_H__

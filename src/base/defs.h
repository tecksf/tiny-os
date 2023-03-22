#ifndef __BASE_DEFS_H__
#define __BASE_DEFS_H__

#ifndef NULL
#define NULL ((void *)0)
#endif

#define true 1
#define false 0

#define __always_inline inline __attribute__((always_inline))
#define __noinline __attribute__((noinline))
#define __noreturn __attribute__((noreturn))

typedef int bool;
typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;
typedef int32 intptr;
typedef uint32 uintptr;
typedef uintptr usize;

#define RoundDown(a, n) ({        \
    uint32 __a = (uint32)(a);     \
    (typeof(a))(__a - __a % (n)); \
})

#define RoundUp(a, n) ({                                \
    uint32 __n = (uint32)(n);                           \
    (typeof(a))(RoundDown((uint32)(a) + __n - 1, __n)); \
})

#define offsetof(type, member) ((uint32)(&((type *)0)->member))

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr)-offsetof(type, member)))

#endif  // __BASE_DEFS_H__

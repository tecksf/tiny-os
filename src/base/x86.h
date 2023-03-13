#ifndef __BASE_X86_H__
#define __BASE_X86_H__

#include "defs.h"

// 该结构用于放入中断符表寄存器IDTR中
struct pseudodesc {
    uint16 pd_lim;        // 16位的表界限
    uintptr pd_base;      // 32位的表基值
} __attribute__ ((packed));


#define do_div(n, base) ({                                        \
    unsigned long __upper, __low, __high, __mod, __base;        \
    __base = (base);                                            \
    asm("" : "=a" (__low), "=d" (__high) : "A" (n));            \
    __upper = __high;                                            \
    if (__high != 0) {                                            \
        __upper = __high % __base;                                \
        __high = __high / __base;                                \
    }                                                            \
    asm("divl %2" : "=a" (__low), "=d" (__mod)                    \
        : "rm" (__base), "0" (__low), "1" (__upper));            \
    asm("" : "=A" (n) : "a" (__low), "d" (__high));                \
    __mod;                                                        \
 })

static inline uint8 inb(uint16 port) __attribute__((always_inline));
static inline void insl(uint32 port, void *addr, int cnt) __attribute__((always_inline));
static inline void outb(uint16 port, uint8 data) __attribute__((always_inline));
static inline void outw(uint16 port, uint16 data) __attribute__((always_inline));
static inline void lidt(struct pseudodesc *pd) __attribute__((always_inline));
static inline void sti(void) __attribute__((always_inline));
static inline void cli(void) __attribute__((always_inline));
static inline uint32 read_ebp(void) __attribute__((always_inline));
static inline uint32 read_eip(void) __attribute__((always_inline));

static inline void lidt(struct pseudodesc *pd)
{
    asm volatile("lidt (%0)" :: "r"(pd) : "memory");
}

static inline void sti(void)
{
    asm volatile("sti");
}

static inline void cli(void)
{
    asm volatile("cli" ::: "memory");
}

static inline void ltr(uint16 sel)
{
    asm volatile ("ltr %0" :: "r" (sel) : "memory");
}

static inline uint32 read_eip(void)
{
    uint32 eip;
    asm volatile("movl 4(%%ebp), %0" : "=r" (eip));
    return eip;
}

static inline uint32 read_ebp(void)
{
    uint32 ebp;
    asm volatile ("movl %%ebp, %0" : "=r" (ebp));
    return ebp;
}

static inline uint32 read_eflags(void)
{
    uint32 eflags;
    asm volatile ("pushfl; popl %0" : "=r" (eflags));
    return eflags;
}

static inline void write_eflags(uint32 eflags)
{
    asm volatile ("pushl %0; popfl" :: "r" (eflags));
}

static inline void lcr0(uintptr cr0)
{
    asm volatile ("mov %0, %%cr0" :: "r" (cr0) : "memory");
}

static inline void lcr3(uintptr cr3)
{
    asm volatile ("mov %0, %%cr3" :: "r" (cr3) : "memory");
}

static inline uintptr rcr0(void)
{
    uintptr cr0;
    asm volatile ("mov %%cr0, %0" : "=r" (cr0) :: "memory");
    return cr0;
}

static inline uintptr rcr1(void)
{
    uintptr cr1;
    asm volatile ("mov %%cr1, %0" : "=r" (cr1) :: "memory");
    return cr1;
}

static inline uintptr rcr2(void)
{
    uintptr cr2;
    asm volatile ("mov %%cr2, %0" : "=r" (cr2) :: "memory");
    return cr2;
}

static inline uintptr rcr3(void)
{
    uintptr cr3;
    asm volatile ("mov %%cr3, %0" : "=r" (cr3) :: "memory");
    return cr3;
}

static inline void invlpg(void *addr)
{
    asm volatile ("invlpg (%0)" :: "r" (addr) : "memory");
}

// 从端口port读取一个字节的数据
static inline uint8 inb(uint16 port)
{
    uint8 data;
    asm volatile("inb %1, %0" : "=a"(data) : "d"(port));
    return data;
}

static inline void insl(uint32 port, void *addr, int cnt)
{
    asm volatile(
            "cld;"
            "repne; insl;"
            : "=D"(addr), "=c"(cnt)
            : "d"(port), "0"(addr), "1"(cnt)
            : "memory", "cc");
}

static inline void outb(uint16 port, uint8 data)
{
    asm volatile("outb %0, %1" ::"a"(data), "d"(port));
}

static inline void outw(uint16 port, uint16 data)
{
    asm volatile("outw %0, %1" ::"a"(data), "d"(port) : "memory");
}

static inline void outsl(uint32 port, const void *addr, int cnt) {
    asm volatile (
            "cld;"
            "repne; outsl;"
            : "=S" (addr), "=c" (cnt)
            : "d" (port), "0" (addr), "1" (cnt)
            : "memory", "cc");
}

#endif // __BASE_X86_H__

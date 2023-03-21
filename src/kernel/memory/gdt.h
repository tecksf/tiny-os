#ifndef __MEMORY_GDT_H__
#define __MEMORY_GDT_H__

/* This file contains the definitions for memory management in our OS. */

/* global segment number */
#define SEG_KTEXT 1
#define SEG_KDATA 2
#define SEG_UTEXT 3
#define SEG_UDATA 4
#define SEG_TSS 5

/* global descriptor numbers */
#define GD_KTEXT ((SEG_KTEXT) << 3) // kernel text
#define GD_KDATA ((SEG_KDATA) << 3) // kernel data
#define GD_UTEXT ((SEG_UTEXT) << 3) // user text
#define GD_UDATA ((SEG_UDATA) << 3) // user data
#define GD_TSS ((SEG_TSS) << 3)     // task segment selector

#define DPL_KERNEL (0)
#define DPL_USER (3)

#define KERNEL_CS ((GD_KTEXT) | DPL_KERNEL)
#define KERNEL_DS ((GD_KDATA) | DPL_KERNEL)
#define USER_CS ((GD_UTEXT) | DPL_USER)
#define USER_DS ((GD_UDATA) | DPL_USER)


#define FL_CF           0x00000001  // Carry Flag
#define FL_PF           0x00000004  // Parity Flag
#define FL_AF           0x00000010  // Auxiliary carry Flag
#define FL_ZF           0x00000040  // Zero Flag
#define FL_SF           0x00000080  // Sign Flag
#define FL_TF           0x00000100  // Trap Flag
#define FL_IF           0x00000200  // Interrupt Flag
#define FL_DF           0x00000400  // Direction Flag
#define FL_OF           0x00000800  // Overflow Flag
#define FL_IOPL_MASK    0x00003000  // I/O Privilege Level bitmask
#define FL_IOPL_0       0x00000000  //   IOPL == 0
#define FL_IOPL_1       0x00001000  //   IOPL == 1
#define FL_IOPL_2       0x00002000  //   IOPL == 2
#define FL_IOPL_3       0x00003000  //   IOPL == 3
#define FL_NT           0x00004000  // Nested Task
#define FL_RF           0x00010000  // Resume Flag
#define FL_VM           0x00020000  // Virtual 8086 mode
#define FL_AC           0x00040000  // Alignment Check
#define FL_VIF          0x00080000  // Virtual Interrupt Flag
#define FL_VIP          0x00100000  // Virtual Interrupt Pending
#define FL_ID           0x00200000  // ID flag

#define STA_X           0x8         // Executable segment
#define STA_E           0x4         // Expand down (non-executable segments)
#define STA_C           0x4         // Conforming code segment (executable only)
#define STA_W           0x2         // Writeable (non-executable segments)
#define STA_R           0x2         // Readable (executable segments)
#define STA_A           0x1         // Accessed

/* System segment type bits */
#define STS_T16A    0x1 // Available 16-bit TSS
#define STS_LDT     0x2 // Local Descriptor Table
#define STS_T16B    0x3 // Busy 16-bit TSS
#define STS_CG16    0x4 // 16-bit Call Gate
#define STS_TG      0x5 // Task Gate / Coum Transmitions
#define STS_IG16    0x6 // 16-bit Interrupt Gate
#define STS_TG16    0x7 // 16-bit Trap Gate
#define STS_T32A    0x9 // Available 32-bit TSS
#define STS_T32B    0xB // Busy 32-bit TSS
#define STS_CG32    0xC // 32-bit Call Gate
#define STS_IG32    0xE // 32-bit Interrupt Gate
#define STS_TG32    0xF // 32-bit Trap Gate

#ifndef __ASSEMBLER__

struct SegmentDescriptor {
    unsigned sd_lim_15_0 : 16;      // low bits of segment limit
    unsigned sd_base_15_0 : 16;     // low bits of segment base address
    unsigned sd_base_23_16 : 8;     // middle bits of segment base address
    unsigned sd_type : 4;           // segment type (see STS_ constants)
    unsigned sd_s : 1;              // 0 = system, 1 = application
    unsigned sd_dpl : 2;            // descriptor Privilege Level
    unsigned sd_p : 1;              // present
    unsigned sd_lim_19_16 : 4;      // high bits of segment limit
    unsigned sd_avl : 1;            // unused (available for software use)
    unsigned sd_rsv1 : 1;           // reserved
    unsigned sd_db : 1;             // 0 = 16-bit segment, 1 = 32-bit segment
    unsigned sd_g : 1;              // granularity: limit scaled by 4K when set
    unsigned sd_base_31_24 : 8;     // high bits of segment base address
};

#define SegmentNull                                                     \
    (struct SegmentDescriptor) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

#define Segment(type, base, lim, dpl)                                   \
    (struct SegmentDescriptor) {                                        \
        ((lim) >> 12) & 0xffff, (base) & 0xffff,                        \
        ((base) >> 16) & 0xff, type, 1, dpl, 1,                         \
        (unsigned)(lim) >> 28, 0, 0, 1, 1,                              \
        (unsigned) (base) >> 24                                         \
    }

#define SegmentTSS(type, base, lim, dpl)                                \
    (struct SegmentDescriptor) {                                        \
        (lim) & 0xffff, (base) & 0xffff,                                \
        ((base) >> 16) & 0xff, type, 0, dpl, 1,                         \
        (unsigned) (lim) >> 16, 0, 0, 1, 0,                             \
        (unsigned) (base) >> 24                                         \
    }

#endif // __ASSEMBLER__

#endif // __MEMORY_GDT_H__

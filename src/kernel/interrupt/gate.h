#ifndef __INTERRUPT_GATE_H__
#define __INTERRUPT_GATE_H__

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

// 中断门描述符
struct GateDescriptor
{
    unsigned gd_off_15_0: 16;  // low 16 bits of offset in segment
    unsigned gd_ss: 16;        // segment selector
    unsigned gd_args: 5;       // # args, 0 for interrupt/trap gates
    unsigned gd_rsv1: 3;       // reserved(should be zero I guess)
    unsigned gd_type: 4;       // type(STS_{TG,IG32,TG32})
    unsigned gd_s: 1;          // must be 0 (system)
    unsigned gd_dpl: 2;        // descriptor(meaning new) privilege level
    unsigned gd_p: 1;          // Present
    unsigned gd_off_31_16: 16; // high bits of offset in segment
};

#define SetGate(gate, is_trap, sel, off, dpl)            \
    {                                                    \
        (gate).gd_off_15_0 = (uint32)(off)&0xffff;       \
        (gate).gd_ss = (sel);                            \
        (gate).gd_args = 0;                              \
        (gate).gd_rsv1 = 0;                              \
        (gate).gd_type = (is_trap) ? STS_TG32 : STS_IG32;\
        (gate).gd_s = 0;                                 \
        (gate).gd_dpl = (dpl);                           \
        (gate).gd_p = 1;                                 \
        (gate).gd_off_31_16 = (uint32)(off) >> 16;       \
    }

#endif // __INTERRUPT_GATE_H__

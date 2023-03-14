#include <x86.h>
#include <string.h>
#include <page.h>

#define LPTPORT 0x378

#define MONO_BASE 0x3B4
#define MONO_BUF 0xB0000
#define CGA_BASE 0x3D4
#define CGA_BUF 0xB8000
#define CRT_ROWS 25
#define CRT_COLS 80
#define CRT_SIZE (CRT_ROWS * CRT_COLS)

static uint16 *crt_buf;
static uint16 crt_pos;
static uint16 addr_6845;

/* stupid I/O delay routine necessitated by historical PC design flaws */
static void delay(void)
{
    inb(0x84);
    inb(0x84);
    inb(0x84);
    inb(0x84);
}

// 采用PIO方式，CPU直接对设备的的IO端口进行操作
static void lpt_putc_sub(int c)
{
    int i;

    // 读IO端口地址0x379，等待并口准备好
    for (i = 0; !(inb(LPTPORT + 1) & 0x80) && i < 12800; i++)
    {
        delay();
    }
    outb(LPTPORT + 0, c);
    outb(LPTPORT + 2, 0x08 | 0x04 | 0x01);
    outb(LPTPORT + 2, 0x08);
}

/* lpt_putc - copy console output to parallel port */
static void lpt_putc(int c)
{
    if (c != '\b')
    {
        lpt_putc_sub(c);
    }
    else
    {
        lpt_putc_sub('\b');
        lpt_putc_sub(' ');
        lpt_putc_sub('\b');
    }
}

// 显示器初始化，CGA 是 Color Graphics Adapter 的缩写
// CGA显存按照下面的方式映射：
//   -- 0xB0000 - 0xB7FFF 单色字符模式
//   -- 0xB8000 - 0xBFFFF 彩色字符模式及 CGA 兼容图形模式
// 6845芯片是IBM PC中的视频控制器
// CPU通过IO地址0x3B4-0x3B5来驱动6845控制单色显示，通过IO地址0x3D4-0x3D5来控制彩色显示。
//    -- 数据寄存器 映射 到 端口 0x3D5或0x3B5
//    -- 索引寄存器 0x3D4或0x3B4,决定在数据寄存器中的数据表示什么。

/* TEXT-mode CGA/VGA display output */
static void cga_init(void)
{
    volatile uint16 *cp = (uint16 *)(CGA_BUF + KERNEL_BASE); //CGA_BUF: 0xB8000 (彩色显示的显存物理基址)
    uint16 was = *cp;                          //保存当前显存0xB8000处的值
    *cp = (uint16)0xA55A;                      // 给这个地址随便写个值，看看能否再读出同样的值
    if (*cp != 0xA55A)
    {                              // 如果读不出来，说明没有这块显存，即是单显配置
        cp = (uint16 *)(MONO_BUF + KERNEL_BASE); //设置为单显的显存基址 MONO_BUF： 0xB0000
        addr_6845 = MONO_BASE;     //设置为单显控制的IO地址，MONO_BASE: 0x3B4
    }
    else
    {                         // 如果读出来了，有这块显存，即是彩显配置
        *cp = was;            //还原原来显存位置的值
        addr_6845 = CGA_BASE; // 设置为彩显控制的IO地址，CGA_BASE: 0x3D4
    }

    // Extract cursor location
    // 6845索引寄存器的index 0x0E（及十进制的14）== 光标位置(高位)
    // 6845索引寄存器的index 0x0F（及十进制的15）== 光标位置(低位)
    // 6845 reg 15 : Cursor Address (Low Byte)
    uint32 pos;
    outb(addr_6845, 14);
    pos = inb(addr_6845 + 1) << 8; //读出了光标位置(高位)
    outb(addr_6845, 15);
    pos |= inb(addr_6845 + 1); //读出了光标位置(低位)

    crt_buf = (uint16 *)cp; //crt_buf是CGA显存起始地址
    crt_pos = pos;            //crt_pos是CGA当前光标位置
}

/* cga_putc - print character to console */
static void cga_putc(int c)
{
    // set black on white
    if (!(c & ~0xFF))
    {
        c |= 0x0700;
    }

    switch (c & 0xff)
    {
        case '\b':
            if (crt_pos > 0)
            {
                crt_pos--;
                crt_buf[crt_pos] = (c & ~0xff) | ' ';
            }
            break;
        case '\n':
            crt_pos += CRT_COLS;
        case '\r':
            crt_pos -= (crt_pos % CRT_COLS);
            break;
        default:
            crt_buf[crt_pos++] = c; // write the character
            break;
    }

    // What is the purpose of this?
    if (crt_pos >= CRT_SIZE)
    {
        int i;
        memmove(crt_buf, crt_buf + CRT_COLS, (CRT_SIZE - CRT_COLS) * sizeof(uint16));
        for (i = CRT_SIZE - CRT_COLS; i < CRT_SIZE; i++)
        {
            crt_buf[i] = 0x0700 | ' ';
        }
        crt_pos -= CRT_COLS;
    }

    // move that little blinky thing
    outb(addr_6845, 14);
    outb(addr_6845 + 1, crt_pos >> 8);
    outb(addr_6845, 15);
    outb(addr_6845 + 1, crt_pos);
}

void cons_init(void)
{
    cga_init();
}

void cons_putc(int c)
{
    lpt_putc(c);
    cga_putc(c);
}
#include <defs.h>
#include <elf.h>
#include <x86.h>

#define SECTION_SIZE        512
#define ELF_HDR             ((struct ElfHeader*)0x10000)

// 等待磁盘准备好才能发指令读数据
static void wait_disk_ready()
{
    /*  从端口0x1F7读取1字节数据，判断0号硬盘的状态
        0xC0 = 1100 0000     0x40 = 0100 0000
        第7位为1，表示磁盘繁忙，第6位为1表示LBA模式
        所以返回 01xx xxxx，代表磁盘已经准备好 */
    while ((inb(0x1F7) & 0xC0) != 0x40);
}

// 读取第secno号扇区，到内存地址dst的位置
static void read_section(void *dst, uint32 secno)
{
    // 1.等待磁盘准备好
    wait_disk_ready();

    // 2.往端口0x1F2发送 要读取的扇区的个数
    outb(0x1F2, 1);

    // 3.给出起始扇区号，一共由28位表示（采用LBA28寻址方式）
    // 0x1f3: 0~7 bits, 0x1f4: 8~15 bits, 0x1f5: 16~23 bits
    // 0x1f6: 低四位表示24~27 bits，第4位表示主从硬盘，高三位111表示LBA模式
    outb(0x1F3, secno & 0xFF);
    outb(0x1F4, (secno >> 8) & 0xFF);
    outb(0x1F5, (secno >> 16) & 0xFF);
    outb(0x1F6, ((secno >> 24) & 0xF) | 0xE0);

    // 4.往端口0x1f7发送0x20命令，代表要读取数据
    outb(0x1F7, 0x20);

    // 5.等待磁盘准备好数据
    wait_disk_ready();

    // 6.把磁盘扇区数据读到指定内存
    insl(0x1F0, dst, SECTION_SIZE / 4);
}

// 从磁盘offset位置，读取count字节数据到start地址
static void read_segment(uintptr start, uint32 count, uint32 offset)
{
    uintptr end = start + count;

    // 磁盘扇区从1开始，第0号扇区为bootloader
    uint32 secno = (offset / SECTION_SIZE) + 1;
    start -= offset % SECTION_SIZE;

    while (start < end)
    {
        read_section((void *) start, secno);
        start += SECTION_SIZE;
        secno++;
    }
}

void boot_main(void)
{
    // 从1号扇区开始，读取8个扇区数据到 ELF_HDR 地址
    // 这8个扇区存着一个elf file header和多个program header
    // 随着内核代码增加，也许不止8个扇区
    read_segment((uintptr)ELF_HDR, SECTION_SIZE * 8, 0);

    if (ELF_HDR->e_magic != ELF_MAGIC)
    {
        goto bad;
    }

    struct ProgramHeader *ph, *eph;
    // 依次读取program header里所记录的段，program header本身已经
    // 在内存里了，可以通过指针访问；而每一个段都需要读取一次硬盘，加载到内存
    ph = (struct ProgramHeader *) ((uintptr)ELF_HDR + ELF_HDR->e_phoff);
    eph = ph + ELF_HDR->e_phnum;
    for (; ph < eph; ph++)
    {
        read_segment(ph->p_vaddr & 0xFFFFFF, ph->p_memsz, ph->p_offset);
    }

    // 跳到内核代码的入口，这里为 kern_entry
    ((void (*)(void)) (ELF_HDR->e_entry & 0xFFFFFF))();

    // 从磁盘读到内存，或跳转到e_entry，都进行 & 0xFFFFFF，这是为了保证内核代码在物理地址的0~16M内，
    // 无论文本段，代码段的虚拟地址是多少

    bad:
    outw(0x8A00, 0x8A00);
    outw(0x8A00, 0x8E00);

    /* do nothing */
    while (1);
}
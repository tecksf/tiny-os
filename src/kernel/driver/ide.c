#include "ide.h"
#include "console.h"
#include <assert.h>
#include <x86.h>
#include <picirq.h>
#include <trap.h>

#define ISA_DATA                0x00
#define ISA_ERROR               0x01
#define ISA_PRECOMP             0x01
#define ISA_CTRL                0x02
#define ISA_SECCNT              0x02
#define ISA_SECTOR              0x03
#define ISA_CYL_LO              0x04
#define ISA_CYL_HI              0x05
#define ISA_SDH                 0x06
#define ISA_COMMAND             0x07
#define ISA_STATUS              0x07

#define IDE_BSY                 0x80
#define IDE_DRDY                0x40
#define IDE_DF                  0x20
#define IDE_DRQ                 0x08
#define IDE_ERR                 0x01

#define IDE_CMD_READ            0x20
#define IDE_CMD_WRITE           0x30
#define IDE_CMD_IDENTIFY        0xEC

#define IDE_IDENT_SECTORS       20
#define IDE_IDENT_MODEL         54
#define IDE_IDENT_CAPABILITIES  98
#define IDE_IDENT_cmd_sets       164
#define IDE_IDENT_MAX_LBA       120
#define IDE_IDENT_MAX_LBA_EXT   200

#define IO_BASE0                0x1F0
#define IO_BASE1                0x170
#define IO_CTRL0                0x3F4
#define IO_CTRL1                0x374

#define SECTION_SIZE            512
#define MAX_IDE                 4
#define MAX_NSECS               128
#define MAX_DISK_NSECS          0x10000000U
#define VALID_IDE(ide_number)        (((ide_number) >= 0) && ((ide_number) < MAX_IDE) && (ide_devices[ide_number].valid))


#define IO_BASE(ide_number) (channels[(ide_number) >> 1].base)
#define IO_CTRL(ide_number) (channels[(ide_number) >> 1].ctrl)

static const struct
{
    unsigned short base; // IDE通道的端口
    unsigned short ctrl; // Control Base
} channels[2] = {
        {IO_BASE0, IO_CTRL0},
        {IO_BASE1, IO_CTRL1},
};

static struct IdeDevice
{
    unsigned char valid;     // 0 or 1 (If Device Really Exists)
    unsigned int sets;       // Commend Sets Supported
    unsigned int size;       // Size in Sectors
    unsigned char model[41]; // Model in String
} ide_devices[MAX_IDE];

static int ide_wait_ready(unsigned short io_base, bool check_error)
{
    int r;
    while ((r = inb(io_base + ISA_STATUS)) & IDE_BSY)
        /* nothing */;

    if (check_error && (r & (IDE_DF | IDE_ERR)) != 0)
    {
        return -1;
    }
    return 0;
}

void ide_init(void)
{
    unsigned short ide_number, io_base;

    // 主板有两个IDE通道，每个通道两个磁盘
    for (ide_number = 0; ide_number < MAX_IDE; ide_number++)
    {
        // 初始化当前还未检测到磁盘
        ide_devices[ide_number].valid = 0;

        // 获取当前磁盘的IDE通道
        // 第一通道的主从硬盘读写端口从0x1f0开始
        // 第二通道的主从硬盘读写端口从0x170开始
        io_base = IO_BASE(ide_number);

        ide_wait_ready(io_base, 0);

        // 选择该通道下的从盘，模式设置位LBA模式
        outb(io_base + ISA_SDH, 0xE0 | ((ide_number & 1) << 4));
        ide_wait_ready(io_base, 0);

        /* step2: send ATA identify command */
        outb(io_base + ISA_COMMAND, IDE_CMD_IDENTIFY);
        ide_wait_ready(io_base, 0);

        /* step3: polling */
        if (inb(io_base + ISA_STATUS) == 0 || ide_wait_ready(io_base, 1) != 0)
        {
            continue;
        }

        ide_devices[ide_number].valid = 1;

        /* read identification space of the device */
        unsigned int buffer[128];
        insl(io_base + ISA_DATA, buffer, sizeof(buffer) / sizeof(unsigned int));

        unsigned char *ident = (unsigned char *) buffer;
        unsigned int sectors;
        unsigned int cmd_sets = *(unsigned int *) (ident + IDE_IDENT_cmd_sets);
        /* device use 48-bits or 28-bits addressing */
        if (cmd_sets & (1 << 26))
        {
            sectors = *(unsigned int *) (ident + IDE_IDENT_MAX_LBA_EXT);
        }
        else
        {
            sectors = *(unsigned int *) (ident + IDE_IDENT_MAX_LBA);
        }
        ide_devices[ide_number].sets = cmd_sets;
        ide_devices[ide_number].size = sectors;

        /* check if supports LBA */
        assert((*(unsigned short *) (ident + IDE_IDENT_CAPABILITIES) & 0x200) != 0);

        unsigned char *model = ide_devices[ide_number].model, *data = ident + IDE_IDENT_MODEL;
        unsigned int i, length = 40;
        for (i = 0; i < length; i += 2)
            model[i] = data[i + 1], model[i + 1] = data[i];

        do
        {
            model[i] = '\0';
        } while (i-- > 0 && model[i] == ' ');

        kernel_print("ide %d: %10u(sectors), '%s'.\n", ide_number, ide_devices[ide_number].size, ide_devices[ide_number].model);
    }

    // 使能IDE硬盘中断
    pic_enable(IRQ_IDE1);
    pic_enable(IRQ_IDE2);
}

bool ide_device_valid(unsigned short ide_number)
{
    return VALID_IDE(ide_number);
}

usize ide_device_size(unsigned short ide_number)
{
    if (ide_device_valid(ide_number))
    {
        return ide_devices[ide_number].size;
    }
    return 0;
}

int ide_read_sections(unsigned short ide_number, uint32 section_number, void *dst, usize num_of_section)
{
    assert(num_of_section <= MAX_NSECS && VALID_IDE(ide_number));
    assert(section_number < MAX_DISK_NSECS && section_number + num_of_section <= MAX_DISK_NSECS);
    unsigned short io_base = IO_BASE(ide_number), io_ctrl = IO_CTRL(ide_number);

    ide_wait_ready(io_base, 0);

    // generate interrupt
    outb(io_ctrl + ISA_CTRL, 0);
    outb(io_base + ISA_SECCNT, num_of_section);
    outb(io_base + ISA_SECTOR, section_number & 0xFF);
    outb(io_base + ISA_CYL_LO, (section_number >> 8) & 0xFF);
    outb(io_base + ISA_CYL_HI, (section_number >> 16) & 0xFF);
    outb(io_base + ISA_SDH, 0xE0 | ((ide_number & 1) << 4) | ((section_number >> 24) & 0xF));
    outb(io_base + ISA_COMMAND, IDE_CMD_READ);

    int ret = 0;
    for (; num_of_section > 0; num_of_section--, dst += SECTION_SIZE)
    {
        if ((ret = ide_wait_ready(io_base, 1)) != 0)
        {
            goto out;
        }
        insl(io_base, dst, SECTION_SIZE / sizeof(uint32));
    }

out:
    return ret;
}

int ide_write_sections(unsigned short ide_number, uint32 section_number, const void *src, usize num_of_section)
{
    assert(num_of_section <= MAX_NSECS && VALID_IDE(ide_number));
    assert(section_number < MAX_DISK_NSECS && section_number + num_of_section <= MAX_DISK_NSECS);
    unsigned short io_base = IO_BASE(ide_number), io_ctrl = IO_CTRL(ide_number);

    ide_wait_ready(io_base, 0);

    // generate interrupt
    outb(io_ctrl + ISA_CTRL, 0);
    outb(io_base + ISA_SECCNT, num_of_section);
    outb(io_base + ISA_SECTOR, section_number & 0xFF);
    outb(io_base + ISA_CYL_LO, (section_number >> 8) & 0xFF);
    outb(io_base + ISA_CYL_HI, (section_number >> 16) & 0xFF);
    outb(io_base + ISA_SDH, 0xE0 | ((ide_number & 1) << 4) | ((section_number >> 24) & 0xF));
    outb(io_base + ISA_COMMAND, IDE_CMD_WRITE);

    int ret = 0;
    for (; num_of_section > 0; num_of_section--, src += SECTION_SIZE)
    {
        if ((ret = ide_wait_ready(io_base, 1)) != 0)
        {
            goto out;
        }
        outsl(io_base, src, SECTION_SIZE / sizeof(uint32));
    }

out:
    return ret;
}


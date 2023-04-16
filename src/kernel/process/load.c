#include <env.h>
#include <elf.h>
#include <assert.h>
#include <console.h>
#include "load.h"
#include <ide.h>

void load_program()
{
    int ret;
    ret = ide_read_sections(2, 0, temp_user_space_start, 8);
    if (ret != 0)
    {
        panic("fail to read user program!");
    }

    struct ElfHeader *elf = (struct ElfHeader *) temp_user_space_start;
    if (elf->e_magic != ELF_MAGIC)
    {
        panic("cannot recognize ELF file");
    }

    kernel_print("========== load user program successfully ==========\n");
}
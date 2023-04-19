#include <env.h>
#include <elf.h>
#include <assert.h>
#include <console.h>
#include "load.h"
#include <ide.h>

void load_program()
{
    char *dest_address = temp_user_space_start;
    uint32 num_sections = 4096 * 256 / 512;

    int ret, section_number = 0;

    while (section_number < num_sections)
    {
        ret = ide_read_sections(2, section_number, dest_address, 8);
        if (ret != 0)
        {
            panic("fail to read user program!");
        }
        section_number += 8;
        dest_address += 4096;
    }

    struct ElfHeader *elf = (struct ElfHeader *) temp_user_space_start;
    if (elf->e_magic != ELF_MAGIC)
    {
        panic("cannot recognize ELF file");
    }

    kernel_print("========== load user program successfully ==========\n");
}
#ifndef __BASE_ELF_H__
#define __BASE_ELF_H__

#include "defs.h"

#define EI_NIDENT   16
#define ELF_MAGIC   0x464C457FU            // "\x7FELF" in little endian

struct ElfHeader {
    uint32 e_magic;     // must equal ELF_MAGIC
    uint8 e_elf[12];
    uint16 e_type;      // 1=relocatable, 2=executable, 3=shared object, 4=core image
    uint16 e_machine;   // 3=x86, 4=68K, etc.
    uint32 e_version;   // file version, always 1
    uint32 e_entry;     // entry point if executable
    uint32 e_phoff;     // file position of program header or 0
    uint32 e_shoff;     // file position of section header or 0
    uint32 e_flags;     // architecture-specific flags, usually 0
    uint16 e_ehsize;    // size of this elf header
    uint16 e_phentsize; // size of an entry in program header
    uint16 e_phnum;     // number of entries in program header or 0
    uint16 e_shentsize; // size of an entry in section header
    uint16 e_shnum;     // number of entries in section header or 0
    uint16 e_shstrndx;  // section number that contains section name strings
};

struct ProgramHeader {
    uint32 p_type;   // loadable code or data, dynamic linking info,etc.
    uint32 p_offset; // file offset of segment
    uint32 p_vaddr;     // virtual address to map segment
    uint32 p_paddr;     // physical address, not used
    uint32 p_filesz; // size of segment in file
    uint32 p_memsz;  // size of segment in memory (bigger if contains bss）
    uint32 p_flags;  // read/write/execute bits
    uint32 p_align;  // required alignment, invariably hardware page size
};

#endif // __BASE_ELF_H__

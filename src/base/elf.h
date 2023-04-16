#ifndef __BASE_ELF_H__
#define __BASE_ELF_H__

#include "defs.h"

#define EI_NIDENT   16
#define ELF_MAGIC   0x464C457FU            // "\x7FELF" in little endian

#define ELF_PT_NULL	                0	                    // 表示该 Program Header 表项无效，没有对应的段
#define ELF_PT_LOAD	                1	                    // 表示该 Program Header 描述的是一个可加载的段，通常包含可执行代码、数据、只读数据等信息
#define ELF_PT_DYNAMIC	            2	                    // 表示该 Program Header 描述的是动态链接信息，通常包含 .dynamic 节的信息
#define ELF_PT_INTERP	            3	                    // 表示该 Program Header 描述的是动态链接器的路径名，通常包含 .interp 节的信息
#define ELF_PT_NOTE	                4	                    // 表示该 Program Header 描述的是一些辅助信息，通常包含一些调试符号信息、版本信息等
#define ELF_PT_SHLIB	            5	                    // 废弃的常量
#define ELF_PT_PHDR	                6	                    // 表示该 Program Header 描述的是 ELF Header 和 Program Header 表本身的信息
#define ELF_PT_TLS	                7	                    // 表示该 Program Header 描述的是 TLS（Thread-Local Storage，线程局部存储）相关的信息
#define ELF_PT_NUM	                8	                    // 表示 Program Header 中的类型数目
#define ELF_PT_LOOS	                0x60000000	            // 操作系统特定的值的开始标志
#define ELF_PT_GNU_EH_FRAME	        0x6474e550	            // GNU C++ 异常处理框架相关信息
#define ELF_PT_GNU_STACK	        0x6474e551	            // 表示栈的可执行性和可增长性，仅在 Linux 中有效
#define ELF_PT_GNU_RELRO	        0x6474e552	            // 表示只读段的动态重定位，仅在 Linux 中有效
#define ELF_PT_LOPROC	            0x70000000	            // 处理器特定的值的开始标志
#define ELF_PT_HIPROC	            0x7fffffff	            // 处理器特定的值的结束标志
#define ELF_PT_LOUSER	            0x80000000	            // 应用程序保留的值的开始标志
#define ELF_PT_HIUSER	            0xffffffff	            // 应用程序保留的值的结束标志


#define ELF_PF_X    1
#define ELF_PF_W    2
#define ELF_PF_R    4

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

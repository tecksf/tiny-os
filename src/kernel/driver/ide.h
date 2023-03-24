#ifndef __DRIVER_IDE_H__
#define __DRIVER_IDE_H__

// Integrated Drive Electronics, IDE

#include <defs.h>

void ide_init(void);
bool ide_device_valid(unsigned short ide_number);
usize ide_device_size(unsigned short ide_number);

int ide_read_sections(unsigned short ide_number, uint32 section_number, void *dst, usize num_of_section);
int ide_write_sections(unsigned short ide_number, uint32 section_number, const void *src, usize num_of_section);

#endif // __DRIVER_IDE_H__

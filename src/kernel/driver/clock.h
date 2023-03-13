#ifndef __DRIVER_CLOCK_H__
#define __DRIVER_CLOCK_H__

#include <defs.h>

#define TICK_NUM 100

extern volatile usize ticks;

void clock_init(void);

#endif //__DRIVER_CLOCK_H__

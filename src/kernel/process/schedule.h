#ifndef __PROCESS_SCHEDULE_H__
#define __PROCESS_SCHEDULE_H__

#include "process.h"

void schedule(void);
void wakeup_process(struct ProcessControlBlock *process);

#endif //__PROCESS_SCHEDULE_H__

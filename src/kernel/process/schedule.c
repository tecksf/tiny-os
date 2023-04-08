#include "schedule.h"
#include <assert.h>
#include <synchronous.h>

void wakeup_process(struct ProcessControlBlock *process)
{
    assert(process->state != PROCESS_ZOMBIE && process->state != PROCESS_RUNNABLE);
    process->state = PROCESS_RUNNABLE;
}

void schedule(void)
{
    bool flag;
    ListEntry *le, *last;
    struct ProcessControlBlock *next = NULL;
    SaveLocalInterrupt(flag);
    {
        current_process->need_reschedule = 0;
        last = (current_process == idle_process) ? &process_list : &(current_process->list_link);
        le = last;
        do
        {
            if ((le = list_next(le)) != &process_list)
            {
                next = OffsetOfProcessControlBlock(le, list_link);
                if (next->state == PROCESS_RUNNABLE)
                {
                    break;
                }
            }
        } while (le != last);

        if (next == NULL || next->state != PROCESS_RUNNABLE)
        {
            next = idle_process;
        }

        next->runs++;
        if (next != current_process)
        {
            run_process(next);
        }
    }
    RestoreLocalInterrupt(flag);
}

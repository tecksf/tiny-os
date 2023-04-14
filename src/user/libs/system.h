#ifndef __USER_SYSTEM_H__
#define __USER_SYSTEM_H__

#include <defs.h>

void __noreturn exit(int error_code);
int fork(void);
int wait(void);
int waitpid(int pid, int *store);
void yield(void);
int kill(int pid);
int getpid(void);

#endif // __USER_SYSTEM_H__

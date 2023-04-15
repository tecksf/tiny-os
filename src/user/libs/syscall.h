#ifndef __USER_SYSCALL_H__
#define __USER_SYSCALL_H__

#include <defs.h>

void __noreturn exit(int error_code);
int fork(void);
int wait(void);
int waitpid(int pid, int *store);
void yield(void);
int kill(int pid);
int getpid(void);

int system_put_char(int c);
int system_page_dir(void);


#endif // __USER_SYSCALL_H__

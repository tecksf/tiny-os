#ifndef __USER_SYSCALL_H__
#define __USER_SYSCALL_H__

int system_exit(int error_code);
int system_fork(void);
int system_wait(int pid, int *store);
int system_yield(void);
int system_kill(int pid);
int system_get_pid(void);
int system_put_char(int c);
int system_page_dir(void);


#endif // __USER_SYSCALL_H__

#ifndef SYSCALL_H
#define SYSCALL_H

#define VALID_SYSCALL 1

//extern void (*system_call_table[VALID_SYSCALL])(void);
int copy_process(long edi,long esi,long ebp,long esp_old,long ebx,long edx,long ecx,long eax,
				long es,long ds,
				long eip,long cs,long eflags,long esp,long ss);
int child_ret_from_fork();
#endif

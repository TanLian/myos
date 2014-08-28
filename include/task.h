#ifndef TASK_H
#define TASK_H
#include <gdtidt.h>
#include <kernel.h>
#include <list.h>
struct TSS
{
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

struct TASK 
{
	struct list_head list;	//构成双向链表
	struct TSS tss;
	struct GDT ldt[2];
	int pid;
	int state;
	int priority;
	int counter;           //运行时间片，初始值为100ms
};

#define TS_RUNNING	0
#define TS_READY	1
#define TS_SLEEPING	2

#define INITIAL_PRIO		200

void scheduler();
void task_init(struct TASK *task, unsigned int eip,struct TASK *old);
void task_run(struct TASK *task,int counter);
void delay(int ms);

extern struct TASK idle_task;
extern struct TASK TASK0;
extern struct TASK *current;

char pid_map[512];
void init_pid_map(char *map,unsigned int size);
int alloc_pid();
void free_pid(unsigned int pid);

struct TASK t;

#define goto_task0() 	__asm__ ("movl %%esp,%%eax\n\t" \
								 "pushl %%ecx\n\t" \
								 "pushl %%eax\n\t" \
								 "pushfl\n\t" \
								 "pushl %%ebx\n\t" \
								 "pushl $1f\n\t" \
								 "iret\n" \
								 "1:\tmovw %%cx,%%ds\n\t" \
								 "movw %%cx,%%es\n\t" \
								 "movw %%cx,%%fs\n\t" \
								 "movw %%cx,%%gs" \
								 ::"b"(USER_CODE_SEL),"c"(USER_DATA_SEL));

#endif

#include <task.h>
#include <kernel.h>
#include <gdtidt.h>
#include <kprintf.h>
#include <mm.h>
#include <list.h>

static unsigned long TASK0_STACK[256] = {0xf};

struct TASK TASK0 = {
	{0,0},
	/* tss */
	{	/* back_link */
		0,
		/* esp0                                    ss0 */
		(unsigned int)&TASK0_STACK+sizeof TASK0_STACK, DATA_SEL, 
		/* esp1 ss1 esp2 ss2 */
		0, 0, 0, 0, 
		/* cr3 */
		0, 
		/* eip eflags */
		0, 0, 
		/* eax ecx edx ebx */
		0, 0, 0, 0,
		/* esp ebp */
		0, 0,
		/* esi edi */
		0, 0, 
		/* es          cs             ds */
		USER_DATA_SEL, USER_CODE_SEL, USER_DATA_SEL, 
		/* ss          fs             gs */
		USER_DATA_SEL, USER_DATA_SEL, USER_DATA_SEL, 
		/* ldt */
		0x20,
		/* trace_bitmap */
		0x00000000
	},
	/* ldt[2] */
	{
		{0xffff,0x0000,0x00,0xfa,0xcf,0x00},      /*默认LDT代码段*/
		{0xffff,0x0000,0x00,0xf2,0xcf,0x00}       /*默认LDT数据段*/
	},
	/* state */
	TS_RUNNING,
	/* priority */
	INITIAL_PRIO,
	//counter
	INITIAL_PRIO,
};

struct TASK idle_task;
struct TASK *current = &TASK0;

void task_init(struct TASK *task, unsigned int eip,struct TASK *old) {
	memcpy(task, old, sizeof(struct TASK));
	task->tss.esp0 = mm_alloc(memman,2*1024)+2*1024;
	task->tss.eip = eip;
	task->tss.eflags = 0x3202;
	task->tss.esp = mm_alloc(memman,2*1024)+2*1024;

	task->pid = alloc_pid();
	task->priority = INITIAL_PRIO;
	task->counter = INITIAL_PRIO;
	task->state = TS_SLEEPING;
	INIT_LIST_HEAD(&(task->list));
}


void task_run(struct TASK *task,int counter)
{
	list_add(&(task->list),&(TASK0.list),TASK0.list.next);
	task->state = TS_READY;
	task->counter = counter;
	task->priority = counter;
}

void delay(int ms)
{
	int x,y,z;
	for(x=0;x<1000;x++)
		for(y=0;y<1000;y++)
			for(z=0;z<100;z++)
				;
}

void init_pid_map(char *map,unsigned int size)
{
	int i;
	for(i = 0;i< size<<3;i++)
	{
		clrb(map,i);
	}
}

int alloc_pid()
{
	int i;
	for(i = 1;i< 512<<3;i++)
	{
		if(testb(pid_map,i) == 0)
		{
			setb(pid_map,i);
			return i;
		}
	}
	return -1;
}

void free_pid(unsigned int pid)
{
	clrb(pid_map,pid);
}

//目标：当链表上有任务执行的时候，不要执行idle_task。当没有任务（任务链表上只有TASK0和idle_task）的时候，就执行idle_task
void scheduler(void) {
	struct TASK *tmp;
	int counter = -1;
	struct TASK *final = &TASK0;
	int task_nums = 1;	//任务链表上的任务数，初始值为1（TASK0）
		
	LIST_EACH(struct TASK *,&TASK0, tmp)
	{
		task_nums++;
		if(tmp->counter > counter && tmp != &idle_task)
		{
			counter = tmp->counter;
			final = tmp;
		}
	}
		
		//printk("%d ",task_nums);
	if(task_nums == 2 && (struct TASK *)TASK0.list.next == &idle_task)	//任务链表上只有TASK0和idle_task
	{
		final = &idle_task;
	}

	//printk("%d ",final->pid);
	if(counter == 0)
		LIST_EACH(struct TASK *,&TASK0, tmp)
		{
			tmp->counter = tmp->priority;
		}
	
	if (final && (final!=current))		//不能重复jmp到同一个任务
	{
		setgdt(3,103,(int)&final->tss,0x0089);
		setgdt(4,15,(int)&final->ldt,0x0082);
		//current->state = TS_READY;
		//final->state = TS_RUNNING;
		current = final;
		__asm__ __volatile__
		("ljmp	$" TSS_SEL_STR ",	$0\n\t");
	}
}

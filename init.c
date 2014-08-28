#include <kprintf.h>
#include <gdtidt.h>
#include <x86.h>
#include <pit.h>
#include <kb.h>
#include <task.h>
#include <hd.h>
#include <fs.h>
#include <timer.h>
#include <tty.h>
#include <mm.h>
#include <systable.h>
#include <fifo.h>

struct TIMER timer1,timer2,timer3;
void do_idle()
{
	for(;;)
	{
		//执行系统调用sys_hlt()，不能执行内核函数hlt()
		//printk("%d   ",get_ticks());
		//printk("2");
		//delay(10);
		sys_hlt();
	}
}



void init()
{
	unsigned int memtotal = memtest(0x00100000,0xffffffff) ;	
	memman = (struct memman *)MEM_MAN_ADDR;
	init_mm(memman);
	mm_free(memman,0x00200000,memtotal-0x00200000);
	
	struct TASK tty_task;
	
	init_gdt();
	init_idt();
	init_pic();
	init_pit(100);   //将中断频率设置为100HZ，即1秒钟产生100次中断
	init_tty();
	
	
	int buf[32];
	fifo_init(&global_fifo,buf,32);
	
	init_kb();

		cmd_len = 0;
		rep_enter = 0;
	
	
	verify_DPT();
	verify_fs();
	verify_dir();	
	printk("#");		

	init_pid_map(pid_map,512);
	INIT_LIST_HEAD(&TASK0.list);
	
	sti();//所有初始化工作完成后，开启中断

	INIT_LIST_HEAD(&first_timer.list);
	list_add(&last_timer.list,&first_timer.list,first_timer.list.next);
		/*set_timer(&timer1,5,500,ONCE);		//5s
		set_timer(&timer2,10,1000,ONCE);		//10s
		set_timer(&timer3,8,800,ONCE);		//8s*/
		
	//setgdt(3,103,(int)&(TASK0.tss),0x0089);
	//setgdt(4,15,(int)&(TASK0.ldt),0x0082);
	__asm__ ("ltrw	%%ax\n\t"::"a"(TSS_SEL));
	__asm__ ("lldt	%%ax\n\t"::"a"(LDT_SEL));


	goto_task0();//这一句之后，就跳到ring3去了。该指令以上的权限与该指令以下的权限完全不一样
	
	//系统调用printc
	//printc('h');
	//printc('e');
	//printc('l');
	//printc('l');
	//printc('o');
	
	//系统调用printf
	//printf("tanlian!");
	

	//闲置任务
	task_init(&idle_task,(unsigned int)do_idle,&TASK0);
	task_run(&idle_task,10);
	
	task_init(&tty_task,(unsigned int)do_task_tty,&TASK0);
	task_run(&tty_task,300);
	

	//get_ticks();
	
	fork();//当前为任务0，即TASK0,pid为0
	/*if(ret)
	{
		printc('e');
	}
	else
		printc('h');
	fork();*/
	
	while(1);
		//printc('l');;
}

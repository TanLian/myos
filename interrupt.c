#include <kprintf.h>
#include <x86.h>
#include <timer.h>
#include <kb.h>
#include <task.h>
#include <fifo.h>

int excehandler()             //默认异常处理函数
{
	printk("exception...\n");
	
	return 1;
}

int inthandler()             //默认中断处理函数
{
	printk("intrrupt...\n");
	/* Send EOI to both master and slave */ 
    outb(0x20,0x20); /* master PIC */
    outb(0xA0,0x20); /* slave PIC */
	
	return 1;
}


		
void do_timer()            //0x20
{
	++ticks;
	
	outb(0x20,0x60); /* master PIC */
	
	if(ticks >= buoy->time && buoy != &last_timer)
	{
		fifo_write(&global_fifo,TIMER_STA+buoy->id);
		list_del(&buoy->list);
		buoy = (struct TIMER *)first_timer.list.next;

	}
	
	cli();
	if(current != &TASK0 && current)
		current->counter -= 10;
	if (! (ticks%1))
		scheduler();
	//printk("%d ",ticks);
	sti();
}

		
void do_kb()			//0x21
{
	outb(0x20,0x61); /* master PIC */
	scan_code = inb(0x60);
	if(scan_code == 0x2a || scan_code == 0x36)
	{
		key_shf = 1;
		outb(0x20,0x61); /* master PIC */
		return;
	}
	else if(scan_code == 0xaa || scan_code == 0xb6)
		key_shf = 0;
	//show_char();
	fifo_write(&global_fifo,scan_code+KB_STA);
}

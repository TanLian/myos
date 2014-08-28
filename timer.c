#include <pit.h>
#include <x86.h>
#include <timer.h>
#include <types.h>
#include <fifo.h>
#include <list.h>

void init_pit(int hz)
{
	ticks = 0;
	//timerfifo = &global_fifo;
	last_timer.id = 100;
	last_timer.time = 0xffffffff;
	last_timer.flags = PERMANENT;
		//last_timer.list.next = &first_timer.list;//����һ����������
		//last_timer.list.prev = &first_timer.list;
		first_timer.time = 0;
	buoy = (struct TIMER *)first_timer.list.next;
	
	outb(0x43,0x34);
	outb(0x40,(1193180/hz)&0xff);
	outb(0x40,(1193180/hz)>>8);
	
	outb(0x21,inb(0x21)&0xfe);  //�򿪶�ʱ���ж�
}

void set_timer(struct TIMER *timer,int id,unsigned int time,TIMER_FLAG flag)
{
	struct TIMER *tmp,*tmp1;
	timer->id = id;
	timer->time = ticks+time;
	//printk("%d ",ticks);
	timer->flags = flag;
	INIT_LIST_HEAD(&timer->list);

	cli();
	LIST_EACH(struct TIMER *,&first_timer,tmp)
	{
		if(tmp->time > timer->time)
		{
			tmp = (struct TIMER *)tmp->list.prev;
			list_add(&(timer->list),&tmp->list,tmp->list.next);  //��֪��Ϊʲô��������û������ȥ
			break;
		}
		
	}
	buoy = (struct TIMER *)first_timer.list.next;
	sti();
}

void cancel_timer(struct TIMER *timer)
{
	/*struct TIMER *tmp1 = buoy;
	struct TIMER *tmp2 = tmp1->next;
	for(;tmp2 != last_timer;tmp2 = tmp2->next,tmp1 = tmp1->next)
		if(timer->id == tmp2->id)
			break;
		
	tmp1->next = tmp2->next;*/
	return;
}
/*
void reset_timer(struct TIMER *timer)				//ʵ����û�뵽�õ����⣬������ȷ�һ�Űɣ�
{
	//struct TIMER *tmp = timer;
	int time = timer->time_copy;
	cancel_timer(timer);
	set_timer(timer,timer->id,time,PERMANENT);
}*/

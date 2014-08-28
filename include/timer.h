#ifndef TIMER_H
#define TIMER_H
#include <fifo.h>
#include <list.h>
volatile unsigned int ticks;		//每10ms该值加1
typedef	enum timer_flag 
{
	PERMANENT,
	ONCE
}TIMER_FLAG;
struct TIMER
{
	struct list_head list;
	int id;
	unsigned int time;
	TIMER_FLAG flags;						//是一次性定时器还是永久性定时器
	struct TIMER *next;		//构成定时器链表
};
struct TIMER first_timer,last_timer;
struct TIMER *buoy;

void set_timer(struct TIMER *timer,int id,unsigned int time,TIMER_FLAG flag);
void cancel_timer(struct TIMER *timer);
void reset_timer(struct TIMER *timer);

//struct FIFO *timerfifo;

#endif
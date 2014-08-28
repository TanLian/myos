#ifndef FIFO_H
#define FIFO_H

#define CURSOR_STA   0
#define CURSOR_COUNT 2
#define	TIMER_STA (CURSOR_STA+CURSOR_COUNT)
#define	TIMER_COUNT 500
#define	KB_STA (TIMER_STA+TIMER_COUNT)
#define	KB_COUNT 256
#define	MOUSE_STA (KB_STA+KB_COUNT)
#define MOUSE_COUNT 256
#define MAX_FIFO_LEN 32
struct FIFO
{
	int *buf;
	unsigned int next_r;
	unsigned int next_w;
	unsigned int size;
	unsigned int count;
};
struct FIFO global_fifo;

void fifo_init(struct FIFO *fifo,int *buf,unsigned int size);
void fifo_write(struct FIFO *fifo,int data);
int fifo_read(struct FIFO *fifo);
int fifo_status(struct FIFO *fifo);


#endif
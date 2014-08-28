#include <fifo.h>

void fifo_init(struct FIFO *fifo,int *buf,unsigned int size)
{
	fifo->buf = buf;
	fifo->size = size;
	fifo->next_r = fifo->next_w = fifo->count = 0;
}

void fifo_write(struct FIFO *fifo,int data)
{
	if(fifo->count >= fifo->size)
		return;
	//printk("%d %d ",fifo->count,fifo->size);
	fifo->buf[fifo->next_w] = data;
	fifo->next_w++;
	if(fifo->next_w == MAX_FIFO_LEN)
		fifo->next_w = 0;
	fifo->count++;
}

int fifo_read(struct FIFO *fifo)
{
	if(fifo->count <= 0)					//没数据时直接退出
		return -1;
	int a = fifo->buf[fifo->next_r];
	fifo->next_r++;
	if(fifo->next_r == MAX_FIFO_LEN)
		fifo->next_r = 0;
	fifo->count--;
	
	return a;
}

int fifo_status(struct FIFO *fifo)
{
	return fifo->count;
}

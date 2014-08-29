#ifndef MM_H
#define MM_H

#include <list.h>
#define MAX_MEM_LEN 7
#define MEM_MAN_ADDR 0x100000		//管理内存结构体的起始位置
struct meminfo
{
	struct list_head list;	//构成双向链表
	unsigned int addr;
	unsigned int size;
	int flag;			//为1表示正在使用，为0表示可分配
};
struct memman
{
	struct meminfo *meminfo[MAX_MEM_LEN];	//[0]表示1K内存，[1]表示2K内存，[2]表示4K内存，[3]~8K，[4]~16K,[5]~32K,[6]~64K
};

int memman_buoy;
struct memman *memman;
void init_mm(struct memman *mem);
unsigned int memtest(unsigned int start, unsigned int end);
unsigned int mm_alloc(struct memman *mem,unsigned int size);
int mm_free(struct memman *mem,unsigned int addr,unsigned int size);
int mm_tidy(struct memman *mem);

//NAYA的内存分布图
/*0x0000~0x8c00 (70个扇区)内核代码
0x80000~0x807ff	(2KB)	IDT
0x80800~0x907ff (64KB)	GDT
0x90800~0xa0000	(62KB)	栈

//可用内存
0x9000~0x7f000
0xa1000~
*/

#endif

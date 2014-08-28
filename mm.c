//unsigned int mm_alloc(struct memman,unsigned int size)
//int mm_free(struct memman,unsigned int addr,unsigned int size)
#include <mm.h>
#include <x86.h>
#define EFLAGS_AC_BIT		0X00040000
#define CRO_CACHE_DISABLE 	0X60000000
#define TRUE     			1
unsigned int memtest_sub(unsigned int start, unsigned int end);
unsigned int memtest(unsigned int start, unsigned int end)
{
char flg486=0; //1 means true
unsigned int eflg,cr0,i;
eflg=read_eflags();
eflg=eflg|EFLAGS_AC_BIT;//AC_BIT=1
write_eflags(eflg);
eflg=read_eflags();//read out eflags again;
if( (eflg & EFLAGS_AC_BIT) != 0) //means cpu is 486 or higher
 {
 flg486=TRUE;
 }

eflg &= ~EFLAGS_AC_BIT;//AC BIT=0;  we use eflags` AC_BIT to know the cpu type 
write_eflags(eflg);

//disable cache 
if(flg486)   					
 {
  cr0 =rcr0();					
  cr0 |= CRO_CACHE_DISABLE;		//disable cache 
  lcr0(cr0);				
 }
 
 i=memtest_sub(start,end);
 
 //open cache
if(flg486)   					
 {
  cr0 =rcr0();					
  cr0 &= ~CRO_CACHE_DISABLE;	//open cache 
  lcr0(cr0);					
 }
 return i;
}


unsigned int memtest_sub(unsigned int start, unsigned int end)
{
	unsigned int i,old;
	volatile unsigned int *p;	//"volatile"这个关键字非常的重要, 你可以尝试下把这个关键字去掉,然后运行,看看结果
						            //可以防止编译器对程序进行了优化 ,如果没有这个关键字,就会出现作者所说的情况,内存大小显示是3072MB
    unsigned int pat0 =0xaa55aa55;	//1010 1010 0101 0101
	unsigned int pat1 = 0x55aa55aa;	//0101 0101 1010 1010
	for(i=start;i<=end; i+=0x1000)//每次检查4k
	{
	  p=(unsigned int *)(i+0xffc);//只检查4k末尾的4个字节
	  old= *p; //save mem value
	  *p= pat0;//writer memory 
	   
	  *p ^= 0xffffffff;// read memory and write memory
	  
	  if(*p != pat1)
	  {
no_memory:
		*p=old;
		break;
	  
	  }
	  *p =old;
	}
	return i;
}

void init_mm(struct memman *mem)
{
	int i;
	struct memman *memm = mem;
	memman_buoy = 0;
	for(i=0;i<MAX_MEM_LEN;i++)
	{
		memm->meminfo[i] = (struct meminfo *)(MEM_MAN_ADDR+sizeof(struct memman)+i*sizeof(struct meminfo));
		memm->meminfo[i]->next = (struct meminfo *)0;
		memm->meminfo[i]->prev = (struct meminfo *)0;
	}
}

struct meminfo *get_first_child(struct memman *mem,struct meminfo *parent)
{
	/*将parent下放（放到下一级，并返回他的第一个孩子）*/
	struct memman *memm = mem;
	struct meminfo *p = parent;
	unsigned int addr = p->addr;
	unsigned int size = (p->size)>>1;
	
	mm_down_meminfo(memm,p);
	
			
	
	int level;
	for(level=1;size >(1<<level)*1024;level++)
		;
	level--;

	struct meminfo *tmp = memm->meminfo[level]->next;
	while(tmp && addr != tmp->addr)
		tmp = tmp->next;
	
			
	return tmp;
}

unsigned int mm_alloc(struct memman *mem,unsigned int size)
{
	struct memman *memm = mem;
	if(size>128*1024)
	{
		int num = size/(128*1024);
		num++;
		struct meminfo *stratmem = memm->meminfo[6]->next;
		int count = 0;
		int found = 0;
			int aa = 20;
			int bb = 40;
		while(stratmem)
		{
			if(stratmem->flag == 0)			
			{
				count++;
				if(count>=num)
				{
					found = 1;
				}
			}
			else
			{
				count = 0;
			}
			if(found == 1)
			{
				int i;
				for(i=0;i<num-1;i++)
				{
					if(stratmem)
					{
						stratmem->flag = 1;
						stratmem = stratmem->prev;
					}
				}
				return stratmem->addr;
			}
			stratmem = stratmem->next;
		}
	}
	//找到所分配的内存属于哪一级，即哪一个链表
	int level;
	int first = 0;		//是否是在当前的level下的链表
	unsigned int sz = size;
	for(level=1;sz>(1<<level)*1024;level++)
		;
	level--;
	
	
	//遍历这一级，看链表上是否有可用内存，即依次查看flag是否为0
retry:	;			//注：不加分号编译不过，不知道为什么
	int flags = 0;
	struct meminfo *checkmem = memm->meminfo[level]->next;
	while(checkmem)
	{
		if(checkmem->flag == 0)			
		{
			flags = 1;
			break;
		}
		checkmem = checkmem->next;
	}
	if(flags)			//找到了这样的一个结构体，将它的flag置为1，返回它的地址
	{
				/*struct point p1 = {0,30};
				char sss[30];
				sprintf(sss,"first = %d",first);
				puts(sss,move_point(p1,P_DOWN,30),5);*/
		if(first == 0)
		{
			checkmem->flag = 1;
			return checkmem->addr;
		}
		else
		{
			struct meminfo *tmp = checkmem;
			while(first > 0)
			{
				tmp = get_first_child(memm,tmp);						
				first--;						
			}
			tmp->flag = 1;
			return tmp->addr;
		}
	}
	level++;		//没有找到这样的一个结构体，前往上一级结构体继续寻找
	if(level >= MAX_MEM_LEN)
		return -1;	//已遍历完所有链表，未找到满足条件的结构体
	first++;
	goto retry;
}



int mm_free(struct memman *mem,unsigned int addr,unsigned int size)
{
	if(size > (1<<MAX_MEM_LEN)*1024)
	{
		int num = size/((1<<MAX_MEM_LEN)*1024);
		int i;
		struct meminfo *tmp = mem->meminfo[MAX_MEM_LEN-1];
		for(i=0;i<num;i++)
		{
			struct meminfo *mi = (struct meminfo *)(MEM_MAN_ADDR+sizeof(struct memman)+MAX_MEM_LEN*sizeof(struct meminfo)+i*sizeof(struct meminfo));
			mi->addr = addr + i*((1<<MAX_MEM_LEN)*1024);
			mi->size = (1<<MAX_MEM_LEN)*1024;
			mi->flag = 0;
			tmp->next = mi;
			mi->prev = tmp;
			mi->next = (void *)0;
			tmp = mi;
			memman_buoy++;
		}

		return 1;
	}
	else
	{
		/*根据addr和size找到那个struct meminfo结构体，将其flag置为0*/
		int level;
		for(level=1;size>(1<<level)*1024 && level<MAX_MEM_LEN;level++)
			;
		level--;
		struct meminfo *mi = mem->meminfo[level]->next;
		while(mi)
		{
			if(mi->addr = addr)
			{
				mi->flag = 0;
				return 1;
			}
			mi = mi->next;
		}
		
		return -1;
	}
}

/*将meminfo增加到对应的内存链表中（小地址在前）*/
void mm_add_to_list(struct memman *mem,struct meminfo *meminfo)
{
	struct meminfo *mi = meminfo;
	int level;
	for(level=1;mi->size>(1<<level)*1024;level++)
		;
	level--;
	
	struct meminfo *tmp = mem->meminfo[level]->next;
	if(tmp == (void *)0)
	{
		//tmp = (struct meminfo *)(MEM_MAN_ADDR+sizeof(struct memman)+MAX_MEM_LEN*sizeof(struct meminfo)+(memman_buoy++)*sizeof(struct meminfo));
		mem->meminfo[level]->next = mi;
		mi->prev = mem->meminfo[level];
		return;
	}
	int islast = 0;
	while(tmp && (tmp->addr < mi->addr))
	{
		if(tmp->next != (void *)0)
			tmp = tmp->next;
		else
		{
			islast = 1;
			break;
		}
	}
	if(islast)
	{
		tmp->next = mi;
		mi->prev = tmp;
		return;
	}
	mi->next = tmp;
	mi->prev = tmp->prev;
	tmp->prev = mi;
	mi->prev->next = mi;

}

void mm_down_meminfo(struct memman *mem,struct meminfo *meminfo)
{
	struct meminfo *mi = meminfo;
	mi->prev->next = mi->next;
	mi->next->prev = mi->prev;	//将该结构体从该链表删除，因为要将他下放到低一级的链表中
	int size = (mi->size)>>1;
			
	struct meminfo *mi1 = (struct meminfo *)(MEM_MAN_ADDR+sizeof(struct memman)+MAX_MEM_LEN*sizeof(struct meminfo)+(memman_buoy++)*sizeof(struct meminfo));
	struct meminfo *mi2 = (struct meminfo *)(MEM_MAN_ADDR+sizeof(struct memman)+MAX_MEM_LEN*sizeof(struct meminfo)+(memman_buoy++)*sizeof(struct meminfo));
	mi1->addr = mi->addr;
	mi2->addr = mi->addr + size;
	mi1->size = size;
	mi2->size = size;
	mi1->flag = mi->flag;
	mi2->flag = mi->flag;
	mm_add_to_list(mem,mi1);
	mm_add_to_list(mem,mi2);
		/*struct point p2 = {0,50};
		char sss2[30];
		sprintf(sss2,"mi2->addr = %d",addr2);
		puts(sss2,move_point(p2,P_DOWN,30),5);*/
}

void mm_up_meminfo(struct memman *mem,struct meminfo *meminfo)
{
	struct meminfo *mi = meminfo;
	
	//将mi和mi->next从该链表剔除
	mi->prev->next = mi->next->next;
	mi->next->next->prev = mi->prev;
	//更改mi->size为两倍，然后添加到上一级链表中
	mi->size = (mi->size)*2;
	mm_add_to_list(mem,mi);
}


int mm_tidy(struct memman *mem)
{
	/*注意，只在特殊时期（如申请的内存无法得到满足时）才调用这个函数，该函数是对内存碎片进行整理*/
	//从第一级链表遍历到最后一级链表，若低级的可合并，就合并到高一级链表中
	//该算法还有点缺陷，如只能对连续两个结构体进行合并，不能对连续的三个进行合并
	int i;
	struct meminfo *mi;
	for(i=0;i<MAX_MEM_LEN-1;i++)
	{
		mi = mem->meminfo[i]->next;
		struct meminfo *tmp = mi;
		while(tmp)
		{
			if((tmp->addr + tmp->size) == tmp->next->addr)
			{
				//可以合并，加入到上一级（size加倍）链表中
				tmp = mi->next->next;
				mm_up_meminfo(mem,mi);
				mi = tmp;
			}
		}
	}
}


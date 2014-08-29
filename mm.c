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
	volatile unsigned int *p;	//"volatile"����ؼ��ַǳ�����Ҫ, ����Գ����°�����ؼ���ȥ��,Ȼ������,�������
						            //���Է�ֹ�������Գ���������Ż� ,���û������ؼ���,�ͻ����������˵�����,�ڴ��С��ʾ��3072MB
    unsigned int pat0 =0xaa55aa55;	//1010 1010 0101 0101
	unsigned int pat1 = 0x55aa55aa;	//0101 0101 1010 1010
	for(i=start;i<=end; i+=0x1000)//ÿ�μ��4k
	{
	  p=(unsigned int *)(i+0xffc);//ֻ���4kĩβ��4���ֽ�
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
		memm->meminfo[i] = (struct meminfo *)(MEM_MAN_ADDR+sizeof(struct memman)+i*sizeof(struct meminfo));//Ϊ��������ͷָ���洢�ռ�
		INIT_LIST_HEAD(&(memm->meminfo[i]->list));
	}
}

struct meminfo *get_first_child(struct memman *mem,struct meminfo *parent)
{
	/*��parent�·ţ��ŵ���һ�������������ĵ�һ�����ӣ�*/
	struct memman *memm = mem;
	struct meminfo *p = parent;
	unsigned int addr = p->addr;
	unsigned int size = (p->size)>>1;
	
	mm_down_meminfo(memm,p);
	
	int level;
	for(level=0;size>(1<<level)*1024 && level<MAX_MEM_LEN;level++)
		;

	struct meminfo *tmp;	
	LIST_EACH(struct meminfo *,memm->meminfo[level], tmp)
	{
		if(addr == tmp->addr)
			break;
	}
	
			
	return tmp;
}

unsigned int mm_alloc(struct memman *mem,unsigned int size)
{
	struct memman *memm = mem;
	if(size>64*1024)
	{
		int num = size/(64*1024);
		int yu = size%(64*1024);
		if(yu)
			num++;
		int count = 0;
		int found = 0;

		struct meminfo *stratmem;
		LIST_EACH(struct meminfo *,memm->meminfo[6], stratmem)
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
						stratmem = (struct meminfo *)(stratmem->list.prev);
					}
				}
				return stratmem->addr;
			}
		}
		return 0;	//δ�ҵ�����128K���ڴ棬����0
	}
	//�ҵ���������ڴ�������һ��������һ������
	int first = 0;		//�Ƿ����ڵ�ǰ��level�µ�����
	unsigned int sz = size;
	int level;
	for(level=0;size>(1<<level)*1024 && level<MAX_MEM_LEN;level++)
		;

	//������һ�������������Ƿ��п����ڴ棬�����β鿴flag�Ƿ�Ϊ0
retry:	;			//ע�����ӷֺű��벻������֪��Ϊʲô
	int flags = 0;
	struct meminfo *checkmem;
	LIST_EACH(struct meminfo *,memm->meminfo[level], checkmem)
	{
		if(checkmem->flag == 0)			
		{
			flags = 1;
			break;
		}
	}
	if(flags)			//�ҵ���������һ���ṹ�壬������flag��Ϊ1���������ĵ�ַ
	{
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
	level++;		//û���ҵ�������һ���ṹ�壬ǰ����һ���ṹ�����Ѱ��
	if(level >= MAX_MEM_LEN)
		return 0;	//�ѱ�������������δ�ҵ����������Ľṹ��
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
			mi->addr = addr + i*((1<<(MAX_MEM_LEN-1))*1024);
			mi->size = (1<<(MAX_MEM_LEN-1))*1024;
			mi->flag = 0;
			list_add(&mi->list,&tmp->list,tmp->list.next);
			tmp = mi;
			memman_buoy++;
		}

		return 1;
	}
	else
	{
		/*����addr��size�ҵ��Ǹ�struct meminfo�ṹ�壬����flag��Ϊ0*/
		int level;
		for(level=0;size>(1<<level)*1024 && level<MAX_MEM_LEN;level++)
			;
		struct meminfo *mi;
		LIST_EACH(struct meminfo *,mem->meminfo[level], mi)
		{
			if(mi->addr = addr)
			{
				mi->flag = 0;
				return 1;
			}
		}
		
		return -1;
	}
}

/*��meminfo���ӵ���Ӧ���ڴ������У�С��ַ��ǰ��*/
void mm_add_to_list(struct memman *mem,struct meminfo *meminfo)
{
	struct meminfo *mi = meminfo;
	int level;
	for(level=0;mi->size>(1<<level)*1024 && level<MAX_MEM_LEN;level++)
		;
	
	struct meminfo *tmp;
	LIST_EACH(struct meminfo *,mem->meminfo[level], tmp)
	{
		if(tmp->addr > mi->addr)
			break;
	}
	tmp = (struct meminfo *)(tmp->list.prev);
	list_add(&mi->list,&tmp->list,tmp->list.next);
}

void mm_down_meminfo(struct memman *mem,struct meminfo *meminfo)
{
	struct meminfo *mi = meminfo;
	list_del(&mi->list);
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
}


//��meminfo����֮���һ�����ӳ�һ��
void mm_up_meminfo(struct memman *mem,struct meminfo *meminfo)
{
	struct meminfo *mi1 = meminfo;
	struct meminfo *mi2 = (struct meminfo *)(meminfo->list.next);
	
	//��mi��mi->next�Ӹ������޳�
	list_del(&mi1->list);
	list_del(&mi2->list);
	
	//����mi->sizeΪ������Ȼ����ӵ���һ��������
	mi1->size = (mi1->size)*2;
	mm_add_to_list(mem,mi1);
}


int mm_tidy(struct memman *mem)
{
	/*ע�⣬ֻ������ʱ�ڣ���������ڴ��޷��õ�����ʱ���ŵ�������������ú����Ƕ��ڴ���Ƭ��������*/
	//�ӵ�һ��������������һ���������ͼ��Ŀɺϲ����ͺϲ�����һ��������
	//���㷨���е�ȱ�ݣ���ֻ�ܶ����������ṹ����кϲ������ܶ��������������кϲ�
	/*int i;
	struct meminfo *mi;
	for(i=0;i<MAX_MEM_LEN-1;i++)
	{
		mi = mem->meminfo[i]->next;
		struct meminfo *tmp = mi;
		while(tmp)
		{
			if((tmp->addr + tmp->size) == tmp->next->addr)
			{
				//���Ժϲ������뵽��һ����size�ӱ���������
				tmp = mi->next->next;
				mm_up_meminfo(mem,mi);
				mi = tmp;
			}
		}
	}*/
}


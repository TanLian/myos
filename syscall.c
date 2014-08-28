#include <syscall.h>
#include <timer.h>
#include <x86.h>
#include <mm.h>	//提供mm_alloc
#include <task.h>	//提高current
#include <kprintf.h>//提供printf
#include <tty.h> //提供haha
#include <list.h>
void run()
{
	while(1)
	{
		printk("1");
		delay(10);
	}
}

void *syscall_entry(int edi,int esi,int ebp,int esp,int ebx,int edx,int ecx,int eax)
{
	int retv = 0;
	switch(edx)    //edx：功能号
	{
		case 1:					//获取ticks
			//printk("1\n");
			retv=ticks;
			break;
		case 2:
			//printk("2\n");
			sys_print(ebx,edi,esi);
			break;
		case 3:
			//printk("3\n");
			return sys_fork();
			break;
		case 4:
			//printk("4\n");
			sys_printf(ebx);
			break;
		case 5:
			hlt();
			break;
		/*case 2:									//memman的初始化
			memman_init((struct MEMMAN *)(ebx + ds_base));
			ecx &= 0xfffffff0;
			memman_free((struct MEMMAN *)(ebx + ds_base),eax,ecx);
			break;
		case 3:									//malloc
			ecx = (ecx + 0x0f) & 0xfffffff0;    //以16字节为单位进位取整
			reg[7] = memman_alloc((struct MEMMAN *)(ebx + ds_base),ecx);
			break;	
		case 4:									//free
			ecx = (ecx + 0x0f) & 0xfffffff0;    //以16字节为单位进位取整
			memman_free((struct MEMMAN *)(ebx + ds_base),eax,ecx);
			break;*/
		default:
			break;
	}
	return retv;
}

void sys_printf(char *ebx)
{
	/*char *s = ebx;
	while(*s)
	{
		print_c(*s,0,0);
		s++;
	}*/
	printk(ebx);
}


int copy_process(long edi,long esi,long ebp,long esp_old,long ebx,long edx,long ecx,long eax,
				long es,long ds,
				long eip,long cs,long eflags,long esp,long ss)
{
	memcpy(&t, current, sizeof(struct TASK));
	t.tss.esp0 = mm_alloc(memman,2*1024)+2*1024;		//202800
	t.tss.eip = (unsigned int)child_ret_from_fork;
	t.tss.eflags = 0x3202;
	t.tss.esp = mm_alloc(memman,2*1024)+2*1024;			//203000
	//复制用户栈数据
	//memcpy(t.tss.esp, current->tss.esp, 1024);
		int *tmpesp = t.tss.esp;
		*tmpesp = ss & 0xff;
		tmpesp--;
		*tmpesp = esp;
		tmpesp--;
		*tmpesp = eflags;
		tmpesp--;
		*tmpesp = cs & 0xff;
		tmpesp--;
		*tmpesp = eip;
		tmpesp--;
		*tmpesp = ds & 0xff;
		tmpesp--;
		*tmpesp = es & 0xff;
		tmpesp--;
		*tmpesp = eax;
		tmpesp--;
		*tmpesp = ecx;
		tmpesp--;
		*tmpesp = edx;
		tmpesp--;
		*tmpesp = ebx;
		tmpesp--;
		*tmpesp = esp_old;
		tmpesp--;
		*tmpesp = ebp;
		tmpesp--;
		*tmpesp = esi;
		tmpesp--;
		*tmpesp = edi;
		t.tss.esp = tmpesp;
		//tmpesp--;
			/*int *data1 = 0x203000;
			printk("%x ",*data1);
			int *data2 = 0x202ffc;
			printk("%x ",*data2);
			int *data3 = 0x202ff8;
			printk("%x ",*data3);
			int *data4 = 0x202ff4;
			printk("%x ",*data4);
			int *data5 = 0x202ff0;
			printk("%x ",*data5);
			
			int *data6 = 0x202800;
			printk("%x ",*data6);
			int *data7 = 0x2027fc;
			printk("%x ",*data7);
			int *data8 = 0x2027f8;
			printk("%x ",*data8);
			int *data9 = 0x2027f4;
			printk("%x ",*data9);
			int *data10 = 0x2027f0;
			printk("%x ",*data10);*/
	t.priority = 10;
	t.counter = 10;		//给它一个很低的优先级
			//printk("%x %x %x %x %x %x %x ",esi,ebp,esp_old,ebx,edx,ecx,eax);
			//printk("%x ",edx);
	t.state = TS_SLEEPING;
	INIT_LIST_HEAD(&(t.list));
	
	t.tss.backlink = 0;
    t.tss.eax = 0;			//这个很关键
	
    t.tss.ecx = ecx;
    t.tss.edx = edx;
    t.tss.ebx = ebx;
    t.tss.ebp = ebp;
    t.tss.esi = esi;
    t.tss.edi = edi;

	
	//current->tss.eax = 2;		//这个很关键，要设为子进程号
	
	task_run(&t,100);
	return 6;					//这个很关键，不知道返回什么
}
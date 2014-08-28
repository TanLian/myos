#include <gdtidt.h>
#include <x86.h>
#include <kernel.h>
#include <task.h>

void setgdt(unsigned int number,unsigned int limit,int base,int access)//sd: selector describe
{
  struct GDT *sd = gdt + number;
  sd->limit_low=limit&0xffff;
  sd->base_low=base &0xffff;
  sd->base_mid=(base>>16)&0xff;
  sd->access_right=access&0xff;
  sd->limit_high=((limit>>16)&0x0f)|((access>>8)&0xf0);//低４位是limt的高位，高４位是访问的权限设置。
  sd->base_high=(base>>24)&0xff;
}

void init_gdt()
{
	int i;
	gdt = (struct GDT *)GDT_ADDR;
	for(i=0;i<8192;i++)
	{
		setgdt(i,0,0,0);
	}
	setgdt(1,0xfffff,0x00000000,0xc09a);     //内核代码段
	setgdt(2,0xfffff,0x00000000,0xc092);     //内核数据段
	setgdt(3,103,(int)&TASK0.tss,0x0089);
	setgdt(4,15,(int)&TASK0.ldt,0x0082);

	
	load_gdtr(0xffff,GDT_ADDR);              //总共有64K

}

void setidt(int number,int offset,int access)//gd: gate describe
{
  struct IDT *gd = idt + number;
  //idt中有32位的offset address
  gd->offset_low=offset & 0xffff;
  gd->offset_high=(offset>>16)&0xffff;
  
  //16位的selector决定了base address
  gd->selector=1*8;
  
  gd->dw_count=(access>>8)&0xff;
  gd->access_right=(char)(access&0xff);

}

void init_idt()
{
	//cli();
	int i;
	idt = (struct IDT *)IDT_ADDR;
	//安装默认的异常处理函数
	for(i=0;i<0x20;i++)
		setidt(i,(int)asm_excehandler,0x008e);
		
	//setidt(0x14,(int)asm_page_error,0x008e);  
	
	//安装默认的中断处理函数
	for(i=0x20;i<256;i++)
		setidt(i,(int)asm_inthandler,0x008e);
	
	setidt(0x20,(int)asm_timer,0x008e);          //中断门，特权级为0，timer
	setidt(0x21,(int)asm_kb,0x008e);             //中断门，特权级为0，keyboard
	setidt(0x80,(int)asm_sys_call,0x00ef); 			//陷阱门，特权级为3
	setidt(0x81,(int)asm_fork,0x00ef); 			//陷阱门，特权级为3
	
	load_idtr(0x7ff,IDT_ADDR);
}

void init_pic()
{
	//往端口20h（主片）或A0h（从片）写入ICW1。
	outb(0x20,0x11);
	outb(0xa0,0x11);	
	//往端口21h（主片）或A1h（从片）写入ICW2。
    outb(0x21,0x20);
    outb(0xa1,0x28);	
	//往端口21h（主片）或A1h（从片）写入ICW3。
    outb(0x21,0x04);
    outb(0xa1,0x02);
	//往端口21h（主片）或A1h（从片）写入ICW4。
    outb(0x21,0x01);
    outb(0xa1,0x01);
	//关中断
    outb(0x21,0xfb);
    outb(0xa1,0xff);

}
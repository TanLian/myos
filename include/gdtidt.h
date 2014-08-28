#ifndef GDTIDT_H
#define GDTIDT_H

struct GDT
{
  short limit_low;
  short base_low; 
  char  base_mid;
  char  access_right;
  char  limit_high;
  char  base_high; 
};
struct GDT *gdt;

struct IDT
{
  short offset_low;
  short selector; 
  char dw_count;
  char access_right;
  short offset_high;
};
struct IDT *idt;

void init_gdt();
void init_idt();
void init_pic();
extern void asm_excehandler();
extern void asm_divide_error();
extern void asm_debug_exception();
extern void asm_breakpoint();
extern void asm_nmi();
extern void asm_overflow();


extern void asm_inthandler();
extern void asm_page_error();
extern void asm_timer();
extern void asm_kb();
extern void asm_sys_call();
extern void asm_fork();
extern void load_gdtr(int limit, int addr);
extern void load_idtr(int limit, int addr);


#endif
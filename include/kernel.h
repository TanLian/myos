#ifndef HEADER_H
#define HEADER_H

#define CODE_SEL 0x08
#define DATA_SEL 0x10

/* code selector in ldt */
#define USER_CODE_SEL	0x07
/* data and stack selector in ldt */
#define USER_DATA_SEL	0x0f

#define IDT_ADDR 0x80000
#define IDT_SIZE 0x800
#define GDT_ADDR (IDT_ADDR+IDT_SIZE)
#define GDT_SIZE	(8*8192)

#define TSS_SEL		0x18
#define	LDT_SEL		0x20

#define TSS_SEL_STR	"0x18"
#define LDT_SEL_STR	"0x20"

#define HD0_ADDR	(GDT_ADDR+GDT_SIZE)
#define HD0_SIZE	(4*8)

#define PAGE_DIR	((HD0_ADDR+HD0_SIZE+(4*1024)-1) & 0xfffff000)
#define PAGE_SIZE	(4*1024)
#define PAGE_TABLE	(PAGE_DIR+PAGE_SIZE)
#define MEMORY_RANGE (4*1024*1024)

int syscall_ret;

#endif
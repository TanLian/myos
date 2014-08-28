#include <tty.h>
#include <x86.h>
#include <kprintf.h>

void init_screen(struct TTY *p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = 0x8000 >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_to_console(p_tty->p_console, nr_tty + '0');
		out_to_console(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
}

int is_current_console(struct CONSOLE *p_console)
{
	return (p_console == &console_table[nr_current_console]);
}

void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

		

void out_to_console(struct CONSOLE *p_console,char key)
{
	unsigned char *p_vmem = (unsigned char *)(0xb8000 + p_console->cursor * 2);
	int i;
	switch(key)
	{
		case '\n':
			if(p_console->cursor < p_console->original_addr +p_console->v_mem_limit - SCREEN_WIDTH)
				p_console->cursor = p_console->original_addr + SCREEN_WIDTH*((p_console->cursor - p_console->original_addr)/SCREEN_WIDTH+1);
			break;
		case '\b':
			if(p_console->cursor > p_console->original_addr)
			{
				p_console->cursor--;
				*(p_vmem-2) = ' ';
				*(p_vmem-1) = 0x07;
			}
			break;
		case '\t':
			for(i=0;i<TAB_WIDTH;i++)
			{
				*p_vmem++ = ' ';
				*p_vmem++ = 0x07;
				p_console->cursor++;
			}
			break;
		default:
			if(p_console->cursor < p_console->original_addr +p_console->v_mem_limit - 1)
			{
					*p_vmem++ = key;
					*p_vmem++ = 0x07;
					p_console->cursor++;
			}
			break;
	}
	
	while(p_console->cursor >= p_console->current_start_addr + SCREEN_SIZE)
		scroll_screen(p_console, SCR_DN);
		
	set_cursor(p_console->cursor);
	set_video_start_addr(p_console->current_start_addr);
}

void out_to_console_by_string(struct CONSOLE *p_console,char *output)
{
	char *p = output;
	while(*p++)
		out_to_console(p_console,*p);
}

void set_video_start_addr(unsigned int addr)
{
	//cli();
	outb(0x3d4, 0xc);
	outb(0x3d5, (addr >> 8) & 0xff);
	outb(0x3d4, 0xd);
	outb(0x3d5, addr & 0xff);
	//sti();
}

void set_cursor(unsigned int position) {
	//cli();
	outb(0x3d4,0x0e);
	outb(0x3d5,(position>>8)&0xff);
	outb(0x3d4,0x0f);
	outb(0x3d5,position&0xff);
	//sti();
}

void scroll_screen(struct CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

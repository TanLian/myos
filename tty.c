#include <tty.h>
#include <kb.h>
#include <libcc.h>
#include <fifo.h>
#include <kernel.h>
#include <hd.h>
#include <fs.h>
#include <kprintf.h>
//#include <mm.h>

#define TTY_FIRST tty_table
#define TTY_END  tty_table + NR_CONSOLES
int xxx;
unsigned int disp_pos = 0;
void init_tty()
{
	struct TTY *p_tty;
	for(p_tty = TTY_FIRST;p_tty < TTY_END;p_tty++)
		tty_init(p_tty);
		
	select_console(0);
}

void do_task_tty()
{	
	xxx = 0;
	struct TTY *p_tty;
	while(1)
	{
		if(Fn == 1 && key_ctl)
			select_console(0);
		else if(Fn == 2 && key_ctl)
			select_console(1);
		else if(Fn == 3 && key_ctl)
			select_console(2);
			
		for(p_tty = TTY_FIRST;p_tty < TTY_END;p_tty++)
		{
			tty_do_read(p_tty);
			if(xxx)
				tty_do_write(p_tty);
		}
	}
}

void tty_init(struct TTY *p_tty)
{	
	//int *tty_buf = (int *)mm_alloc(memman,128);
	int tty_buf[128];
	//printk("%x ",tty_buf);
	fifo_init(&p_tty->fifo,tty_buf,128>>2);
	init_screen(p_tty);
}

void tty_do_read(struct TTY *p_tty)
{
	xxx = 0;
	if(is_current_console(p_tty->p_console))
	{
		keyboard_read(p_tty);
		xxx = 1;
	}
}

void tty_do_write(struct TTY *p_tty)
{
	int i;
	char output;
	
	if(fifo_status(&p_tty->fifo))
	{
		i = fifo_read(&p_tty->fifo);
		output = key_map[i-KB_STA][key_shf];	
		
		if(output && output != '\n' && output != '\b')
		{
			cmd_line[cmd_len++] = output;
			out_to_console(p_tty->p_console,output);
			rep_enter = 1;
		}
		else if(key_enter == 1)
		{
			if(strncmp(cmd_line,"ls",cmd_len) == 0 && rep_enter == 1)
			{
				struct SUPER_BLOCK sb;
				char sect[512] = {0,};
				//填充超级块sb
				sb.sb_start = *(unsigned int *)(HD0_ADDR);
				hd_rw(ABS_SUPER_BLK(sb), HD_READ, 1, sect);
				memcpy(&sb, sect, sizeof(struct SUPER_BLOCK));
				
				stat(current_inode);
				
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				//printk("syscall_ret:%d\n",syscall_ret);
				out_to_console(p_tty->p_console,'#');
			}
			else if(strncmp(cmd_line,"touch",5) == 0 && rep_enter == 1)
			{
				char *c = cmd_line + 5;
				while(*c == ' ')
				{
					c++;
				}
				if(*c)
				{
					touch(current_inode,c);
				}
				
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				out_to_console(p_tty->p_console,'#');
			}
			else if(strncmp(cmd_line,"fhave",5) == 0 && rep_enter == 1)
			{
				printk("\n");
				char *c = cmd_line + 5;
				while(*c == ' ')
				{
					c++;
				}
				if(*c)
				{
					int result = fhave(current_inode,c);
					if(result != -1)
						printk("have!");
					else
						printk("don't have!");
				}
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				out_to_console(p_tty->p_console,'#');
			}
			else if(strncmp(cmd_line,"cat",3) == 0 && rep_enter == 1)
			{
				printk("\n");
				char *c = cmd_line + 3;
				while(*c == ' ')
				{
					c++;
				}
				if(*c)
				{
					fcat(c);
				}
				
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				out_to_console(p_tty->p_console,'#');
			}
			else if(strncmp(cmd_line,"write",5) == 0 && rep_enter == 1)
			{
				char content[60];
				int i = 0;
				char *c = cmd_line+5;
				while(*c == ' ')
					c++;
				if(*c == '\"')
					c++;
				while(*c != '\"')
				{
					content[i++] = *c;
					c++;
				}
				content[i] = '\0';
				
				c++;	//通过"
				while(*c == ' ' || *c == '>')
				{
					c++;
				}
				write(c,content);
				
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				out_to_console(p_tty->p_console,'#');
			}
			else if(strncmp(cmd_line,"rm",2) == 0 && rep_enter == 1)
			{
				char *c = cmd_line + 2;
				//吃掉空格
				while(*c == ' ')
				{
					c++;
				}
				if(*c)
				{
					rm_file(current_inode,c);
				}
				
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				out_to_console(p_tty->p_console,'#');
			}
			else if(strncmp(cmd_line,"mkdir",5) == 0 && rep_enter == 1)
			{
				char *c = cmd_line + 5;
				//吃掉空格
				while(*c == ' ')
				{
					c++;
				}
				if(*c)
				{
					mkdir(current_inode,c);
				}
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				out_to_console(p_tty->p_console,'#');
			}
			else if(strncmp(cmd_line,"cd",2) == 0 && rep_enter == 1)
			{
				char *c = cmd_line + 2;
				//吃掉空格
				while(*c == ' ')
				{
					c++;
				}
				if(*c)
				{
					cd(c);
				}
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				out_to_console(p_tty->p_console,'#');
			}
			else if(strncmp(cmd_line,"cp",2) == 0 && rep_enter == 1)
			{
				char filename[MAX_NAME_LEN] = {'0',};
				char dirname[MAX_NAME_LEN] = {'0',};
				int i = 0;
				char *c = cmd_line + 2;
				//吃掉空格
				while(*c == ' ')
				{
					c++;
				}
				while(*c != ' ')
				{
					filename[i++] = *c;
					c++;
				}
				filename[i] = '\0';
				while(*c == ' ')
				{
					c++;
				}
				i = 0;
				while(*c != '\/')
				{
					dirname[i++] = *c;
					c++;
				}
				dirname[i] = '\0';
				cp(filename,dirname);
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				out_to_console(p_tty->p_console,'#');
			}
			else if(strncmp(cmd_line,"mv",2) == 0 && rep_enter == 1)
			{
				char filename[MAX_NAME_LEN] = {'0',};
				char dirname[MAX_NAME_LEN] = {'0',};
				int i = 0;
				char *c = cmd_line + 2;
				//吃掉空格
				while(*c == ' ')
				{
					c++;
				}
				while(*c != ' ')
				{
					filename[i++] = *c;
					c++;
				}
				filename[i] = '\0';
				while(*c == ' ')
				{
					c++;
				}
				i = 0;
				while(*c != '\/')
				{
					dirname[i++] = *c;
					c++;
				}
				dirname[i] = '\0';
				mv(filename,dirname);
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				out_to_console(p_tty->p_console,'#');
			}
			else if(strncmp(cmd_line,"sysret",6) == 0 && rep_enter == 1)
				printk("ret:%x\n",syscall_ret);
			else if(cmd_len > 0 && rep_enter == 1)
			{
				printk("\n");
				printk("Unknown command!");
				out_to_console(p_tty->p_console,'\n');
				if(PS1.clen > 0)
					putsPS1();
				else
					printk("/");
				out_to_console(p_tty->p_console,'#');
			}
			
			//printk("cmd_len:%d ",cmd_len);
			for(i = 0;i < cmd_len;i++)
				cmd_line[i] = '\0';
			cmd_len = 0;
			key_enter = 0;
			rep_enter = 0;
		}
		else if(key_backspace == 1)
		{
			cmd_line[cmd_len] = '\0';
			out_to_console(p_tty->p_console,'\b');
			cmd_len--;
			key_backspace = 0;
		}		

	}
}


void keyboard_read(struct TTY* p_tty)
{
	int i;
	if(fifo_status(&global_fifo))
	{
		i=fifo_read(&global_fifo);
		if(i>=KB_STA && i<=KB_STA+KB_COUNT)
		{
			switch(i-KB_STA)
			{
			case 0x0e:                      //BACKSPACE被按下
				key_backspace = 1;
				break;
			case 0x8e:                      //BACKSPACE松开
				key_backspace = 0;
				break;
			case 0x1c:						//ENTER被按下
				key_enter = 1;
				break;
			case 0x9c:						//ENTER松开
				key_enter = 0;
				break;
			case 0x1d:						//左CTL按下
				key_ctl |= 1;
				break;
			case 0x9d:						//左CTL松开
				key_ctl &= ~1;
				break;
			case 0x2a:						//左SHIFT按下
				key_shf |= 1;
				break;
			case 0x36:						//右SHIFT按下
				key_shf |= 2;
				break;
			case 0xaa:						//左SHIFT松开
				key_shf &= ~1;
				break;
			case 0xb6:						//右SHIFT松开
				key_shf &= ~2;
				break;
			case 0x3b:						//F1按下
				Fn = 1;
				break;
			case 0x3c:						//F2
				Fn = 2;
				break;
			case 0x3d:						//F3
				Fn = 3;
				break;
			case 0x3e:						//F4
				Fn = 4;
				break;
			case 0x3f:						//F5
				Fn = 5;
				break;
			case 0x40:						//F6
				Fn = 6;
				break;
			case 0x41:						//F7
				Fn = 7;
				break;
			case 0x42:						//F8
				Fn = 8;
				break;
			case 0x43:						//F9
				Fn = 9;
				break;
			case 0x44:						//F10
				Fn = 10;
				break;
			case 0xbb:						//F1松开
				Fn = 0;
				break;
			case 0xbc:						//F2
				Fn = 0;
				break;
			case 0xbd:						//F3
				Fn = 0;
				break;
			default:
				break;
			}
			fifo_write(&p_tty->fifo, i);			
		}
		else if(i >= TIMER_STA && i <= TIMER_STA+KB_COUNT)	//定时器
				printk("%d   ",i-TIMER_STA);
	}
}

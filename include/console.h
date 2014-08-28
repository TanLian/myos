#ifndef CONSOLE_H
#define CONSOLE_H

#define NR_CONSOLES 3
#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80
struct CONSOLE
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */
};
struct CONSOLE console_table[NR_CONSOLES];
unsigned int nr_current_console;

extern void select_console(int);
extern void set_video_start_addr(unsigned int);
extern int is_current_console(struct CONSOLE *);
//extern void scroll_screen(struct CONSOLE*, int);
extern void set_cursor(unsigned int);
extern void out_to_console(struct CONSOLE *,char);
extern void out_to_console_by_string(struct CONSOLE *,char *);

#endif

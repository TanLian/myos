#ifndef TTY_H
#define TTY_H

#include <console.h>
#include <fifo.h>
#define TTY_MAX_BYTES 256

struct TTY
{
	struct FIFO fifo;
	struct CONSOLE *p_console;
};
struct TTY tty_table[NR_CONSOLES];

extern unsigned int disp_pos;
int csr_x;
int csr_y;

void init_tty();
extern void do_task_tty();
extern void tty_init(struct TTY *);
extern void tty_do_read(struct TTY *);
extern void tty_do_write(struct TTY *);
extern void keyboard_read(struct TTY *);
extern void init_screen(struct TTY *);

#endif
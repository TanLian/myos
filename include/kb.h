#ifndef KB_H
#define KB_H
#include <fifo.h>

extern unsigned char key_map[0x3a][2];	
//struct FIFO *keyfifo;
unsigned int scan_code;
int key_shf;
//int key_led;
int key_ctl;
unsigned short Fn;
int key_enter;
int key_backspace;

char cmd_line[50];
int cmd_len;

void init_kb();


#define KEYCMD_LED		0xed

#endif
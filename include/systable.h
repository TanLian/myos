#ifndef SYSTABLE_H
#define SYSTABLE_H

int get_ticks();
int fork();
int printc(const char c);
int printf(const char *fmt, ...);
void sys_hlt();

#endif
/* Skelix by Xiaoming Mo (skelixos@gmail.com)
 * Licence: GPLv2 */
#ifndef LIBCC_H
#define LIBCC_H
#include <x86.h>
//#define NULL	((void *)0)

void bcopy(const void *src, void *dest, unsigned int n);
void bzero(void *dest, unsigned int n);
void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *dest, int c, unsigned int n);
int memcmp(const void *s1, const void *s2, unsigned int n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2,unsigned int len);
char *strcpy(char *dest, const char *src);
unsigned int strlen(const char *s);

#define assert(s) do { if (! (s)) {	\
kprintf(KPL_PANIC, "ASSERT: %s, %d", __FILE__, __LINE__);	\
hlt();	\
}} while (0)

#endif

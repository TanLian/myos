/* Skelix by Xiaoming Mo (skelixos@gmail.com)
 * Licence: GPLv2 */
#ifndef KPRINTF_H
#define KPRINTF_H


enum KP_LEVEL {KPL_DUMP, KPL_PANIC};
#define TAB_WIDTH	8			/* must be power of 2 */

void kprintf(enum KP_LEVEL, const char *fmt, ...);
void printk(const char *fmt, ...);

typedef enum COLOUR_TAG {
	BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, WHITE,
	GRAY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, 
	LIGHT_RED, LIGHT_MAGENTA, YELLOW, BRIGHT_WHITE
} COLOUR;

#endif

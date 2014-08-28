#include <systable.h>
#include <kernel.h>

int get_ticks()
{
	//input为功能号
	int input = 1,output;
	__asm__ __volatile__
	(
		"int $0x80"
		:"=a"(output):"d"(input)
	);
	return output;
}

/*调用一次fork，返回两次，那两次都是什么时候返回的？*/
int fork()
{
	__asm__ __volatile__
	(
		"int $0x81"
	);
}

int printc(const char c)
{
	//input为功能号
	int input = 2,output;
	__asm__ __volatile__
	(
		"int $0x80"
		:"=a"(output):"d"(input),"b"(c)
	);
	return output;
}

int printf(const char *fmt, ...)
{
	//input为功能号
	int input = 4,output;
	__asm__ __volatile__
	(
		"int $0x80"
		:"=a"(output):"d"(input),"b"(fmt)
	);
	return output;
}

void sys_hlt()
{
	//input为功能号
	int input = 5;
	__asm__ __volatile__
	(
		"int $0x80"
		::"d"(input)
	);
}

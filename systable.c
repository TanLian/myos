#include <systable.h>
#include <kernel.h>

int get_ticks()
{
	//inputΪ���ܺ�
	int input = 1,output;
	__asm__ __volatile__
	(
		"int $0x80"
		:"=a"(output):"d"(input)
	);
	return output;
}

/*����һ��fork���������Σ������ζ���ʲôʱ�򷵻صģ�*/
int fork()
{
	__asm__ __volatile__
	(
		"int $0x81"
	);
}

int printc(const char c)
{
	//inputΪ���ܺ�
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
	//inputΪ���ܺ�
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
	//inputΪ���ܺ�
	int input = 5;
	__asm__ __volatile__
	(
		"int $0x80"
		::"d"(input)
	);
}

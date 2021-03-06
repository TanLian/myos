#include <kb.h>
#include <x86.h>
#include <fifo.h>

unsigned char key_map[0x3a][2] = 
{
	/*00*/{0x0, 0x0}, {0x0, 0x0}, {'1', '!'}, {'2', '@'}, 
	/*04*/{'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'}, 
	/*08*/{'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'},
	/*0c*/{'-', '_'}, {'=', '+'}, {'\b','\b'},{'\t','\t'},
	/*10*/{'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'},
	/*14*/{'t', 'T'}, {'y', 'Y'}, {'u', 'U'}, {'i', 'I'},
	/*18*/{'o', 'O'}, {'p', 'P'}, {'[', '{'}, {']', '}'},
	/*1c*/{'\n','\n'},{0x0, 0x0}, {'a', 'A'}, {'s', 'S'},
	/*20*/{'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'},
	/*24*/{'j', 'J'}, {'k', 'K'}, {'l', 'L'}, {';', ':'},
	/*28*/{'\'','\"'},{'`', '~'}, {0x0, 0x0}, {'\\','|'}, 
	/*2c*/{'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, 
	/*30*/{'b', 'B'}, {'n', 'N'}, {'m', 'M'}, {',', '<'},
	/*34*/{'.', '>'}, {'/', '?'}, {0x0, 0x0}, {'*', '*'},
	/*38*/{0x0, 0x0}, {' ', ' '} 
};

void init_kb()
{
	//keyfifo = &global_fifo;
	key_shf = 0;
	key_ctl = 0;
	Fn = 0;
		
	outb(0x21,inb(0x21)&0xfd);    //�򿪼����ж�
}

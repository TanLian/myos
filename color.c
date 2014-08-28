
void
color(void) {

	__asm__ ("int	$0x80"::"S"(0),"D"(0),"b"('T'),"d"(2));
	while(1);
}

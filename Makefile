AS=as -Iinclude
LD=ld
CC=gcc
CFLAGS=-Wall -pedantic -W -nostdlib -nostdinc -Wno-long-long -I include -fomit-frame-pointer

KERNEL_OBJS= loader.o init.o kprintf.o libcc.o gdtidt.o isr.o interrupt.o timer.o kb.o task.o hd.o fs.o syscall.o fifo.o console.o tty.o mm.o systable.o

.s.o:
	${AS} -a $< -o $*.o >$*.map

all: final.img color

final.img: bootsect system
	cat bootsect system > final.img
	@wc -c final.img

color: color.o
	${LD} --oformat binary -N -e color -Ttext 0x100000 -o color $<
	
bootsect: bootsect.o
	${LD} --oformat binary -N -e start -Ttext 0x7c00 -o bootsect $<

system: ${KERNEL_OBJS}
	${LD} --oformat binary -N -e pm_mode -Ttext 0x0000 -o $@ ${KERNEL_OBJS}
	@wc -c system

*.o: *.c
	$(CC) -nostdinc $(CFLAGS) -Os -c -o $@ $<

clean:
	rm -f *.img system bootsect *.o


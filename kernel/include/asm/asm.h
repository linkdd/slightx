#ifndef __ASM_H
#define __ASM_H

#include <sys/io.h>

void printk (char *format, ...);

#define cli()    asm ("cli")
#define sti()    asm ("sti")

static void reboot (void)
{
	cli ();
	outb (0x64, 0xFE);
	sti ();
}

static void halt (void)
{
	printk ("System halted.\n");
	asm ("hlt");
}

#endif /* __ASM_H */

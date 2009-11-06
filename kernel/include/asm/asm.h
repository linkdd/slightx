#ifndef __ASM_H
#define __ASM_H

#include <sys/io.h>

#define cli()    asm ("cli")
#define sti()    asm ("sti")

static inline void reboot (void)
{
	cli ();
	outb (0x64, 0xFE);
	sti ();
}

#endif /* __ASM_H */

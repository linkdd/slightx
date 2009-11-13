#ifndef __ASM_H
#define __ASM_H

#include <sys/io.h>

void printk (char *format, ...);

#define cli()    asm ("cli")
#define sti()    asm ("sti")

#define reboot()        \
do {                    \
    cli ();             \
    outb (0x64, 0xFE);  \
    sti ();             \
} while (/* CONSTCOND */ 0)

#define halt()                      \
do {                                \
    printk ("System halted.\n");    \
    asm ("hlt");                    \
} while (/* CONSTCOND */ 0)

#endif /* __ASM_H */

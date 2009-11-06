#ifndef __IO_H
#define __IO_H

unsigned char inb (unsigned short port);
unsigned short inw (unsigned short port);
unsigned int ind (unsigned short port);

void outb (unsigned short port, unsigned char value);
void outw (unsigned short port, unsigned short value);
void outd (unsigned short port, unsigned int value);

void iowait (void);

#endif /* __IO_H */

#include <sys/io.h>

unsigned char inb (unsigned short port)
{
	unsigned char toret;
	asm ("inb %1, %0" : "=a"(toret) : "Nd"(port));
	return toret;
}

unsigned short inw (unsigned short port)
{
	unsigned short toret;
	asm ("inw %1, %0" : "=a"(toret) : "Nd"(port));
	return toret;
}

unsigned int ind (unsigned short port)
{
	unsigned int toret;
	asm ("inl %1, %0" : "=a"(toret) : "Nd"(port));
	return toret;
}

void outb (unsigned short port, unsigned char value)
{
	asm ("outb %0, %1" :: "a"(value), "Nd"(port));
	return;
}

void outw (unsigned short port, unsigned short value)
{
	asm ("outw %0, %1" :: "a"(value), "Nd"(port));
	return;
}

void outd (unsigned short port, unsigned int value)
{
	asm ("outl %0, %1" :: "a"(value), "Nd"(port));
	return;
}

void iowait (void)
{
	outb (0x80, 0);
	return;
}

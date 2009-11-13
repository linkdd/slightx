#include <sys/io.h>

uint8_t inb (uint16_t port)
{
    uint8_t toret;
    asm ("inb %1, %0" : "=a"(toret) : "Nd"(port));
    return toret;
}

uint16_t inw (uint16_t port)
{
    uint16_t toret;
    asm ("inw %1, %0" : "=a"(toret) : "Nd"(port));
    return toret;
}

uint32_t ind (uint16_t port)
{
    uint32_t toret;
    asm ("inl %1, %0" : "=a"(toret) : "Nd"(port));
    return toret;
}

void outb (uint16_t port, uint8_t value)
{
    asm ("outb %0, %1" :: "a"(value), "Nd"(port));
    return;
}

void outw (uint16_t port, uint16_t value)
{
    asm ("outw %0, %1" :: "a"(value), "Nd"(port));
    return;
}

void outd (uint16_t port, uint32_t value)
{
    asm ("outl %0, %1" :: "a"(value), "Nd"(port));
    return;
}

void iowait (void)
{
    outb (0x80, 0);
    return;
}

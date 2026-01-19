#include <kernel/cpu/ioport.h>


u8 inb(u16 port) {
  u8 ret;
  asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}


u16 inw(u16 port) {
  u16 ret;
  asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}


u32 inl(u16 port) {
  u32 ret;
  asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}


void outb(u16 port, u8 val) {
  asm volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}


void outw(u16 port, u16 val) {
  asm volatile ("outw %0, %1" :: "a"(val), "Nd"(port));
}


void outl(u16 port, u32 val) {
  asm volatile ("outl %0, %1" :: "a"(val), "Nd"(port));
}


void iowait(void) {
  outb(0x80, 0);
}

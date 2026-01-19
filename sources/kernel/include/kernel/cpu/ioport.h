#pragma once

#include <klibc/types.h>


u8  inb(u16 port);
u16 inw(u16 port);
u32 inl(u16 port);

void outb(u16 port, u8  val);
void outw(u16 port, u16 val);
void outl(u16 port, u32 val);

void iowait(void);

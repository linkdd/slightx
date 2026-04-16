#include <kernel/drivers/serial.h>
#include <kernel/cpu/ioport.h>


void serial_putc(char c) {
  outb(0x3F8, (u8)c);
}


void serial_puts(str s) {
  for (usize i = 0; i < s.length; i++) {
    serial_putc(s.data[i]);
  }
}

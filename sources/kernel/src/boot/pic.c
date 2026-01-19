#include <kernel/boot/pic.h>
#include <kernel/cpu/ioport.h>


void pic_load(void) {
  pic_remap(PIC_MASTER_OFFSET, PIC_SLAVE_OFFSET);
  pic_disable();
}


void pic_irq_set_mask(u8 irq) {
  u16 port;
  u8  value;

  if (irq < 8) {
    port = PIC_MASTER_DATA;
  }
  else {
    port = PIC_SLAVE_DATA;
    irq -= 8;
  }

  value = inb(port) | (1 << irq);
  outb(port, value);
}


void pic_irq_clear_mask(u8 irq) {
  u16 port;
  u8  value;

  if (irq < 8) {
    port = PIC_MASTER_DATA;
  }
  else {
    port = PIC_SLAVE_DATA;
    irq -= 8;
  }

  value = inb(port) & ~(1 << irq);
  outb(port, value);
}


void pic_disable(void) {
  outb(PIC_SLAVE_DATA,  0xFF);
  outb(PIC_MASTER_DATA, 0xFF);
}

void pic_remap(u8 offset_master, u8 offset_slave) {
  u8 master_mask;
  u8 slave_mask;

  master_mask = inb(PIC_MASTER_DATA); iowait();
  slave_mask  = inb(PIC_SLAVE_DATA);  iowait();

  outb(PIC_MASTER_CMD, ICW1_INIT | ICW1_ICW4); iowait();
  outb(PIC_SLAVE_CMD,  ICW1_INIT | ICW1_ICW4); iowait();

  outb(PIC_MASTER_DATA, offset_master); iowait();
  outb(PIC_SLAVE_DATA,  offset_slave);  iowait();

  outb(PIC_MASTER_DATA, 4); iowait();
  outb(PIC_SLAVE_DATA,  2); iowait();

  outb(PIC_MASTER_DATA, ICW4_8086); iowait();
  outb(PIC_SLAVE_DATA,  ICW4_8086); iowait();

  outb(PIC_MASTER_DATA, master_mask); iowait();
  outb(PIC_SLAVE_DATA,  slave_mask);  iowait();
}


void pic_eoi(u8 irq) {
  outb(PIC_MASTER_CMD, PIC_EOI); iowait();

  if (irq >= 8) {
    outb(PIC_SLAVE_CMD, PIC_EOI); iowait();
  }
}

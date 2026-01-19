#include <kernel/boot/idt.h>
#include <kernel/boot/irq.h>
#include <kernel/boot/pic.h>
#include <kernel/halt.h>

#include <klibc/mem/bytes.h>
#include <klibc/sync/lock.h>


static interrupt_cb irq_callbacks[IDT_NUM_IRQS] = {};
static spinlock     irq_lock                    = {};


void irq_init(void) {
  spinlock_init(&irq_lock);
}


void irq_handler(interrupt_frame *iframe) {
  u64 irq = iframe->interrupt - IDT_NUM_EXCEPTIONS;

  interrupt_cb          cb = irq_get_callback(irq);
  interrupt_controlflow cf = INTERRUPT_CONTROLFLOW_RETURN;

  if (cb != NULL) {
    cb(iframe, &cf);
  }

  pic_eoi(irq);

  if (cf == INTERRUPT_CONTROLFLOW_HALT) {
    asm("cli");
    halt();
  }
}


interrupt_cb irq_get_callback(u8 irq) {
  spinlock_acquire(&irq_lock);
  interrupt_cb cb = irq_callbacks[irq];
  spinlock_release(&irq_lock);
  return cb;
}


void irq_set_callback(u8 irq, interrupt_cb cb) {
  spinlock_acquire(&irq_lock);
  irq_callbacks[irq] = cb;
  pic_irq_clear_mask(irq);
  spinlock_release(&irq_lock);
}


void irq_clear_callback(u8 irq) {
  spinlock_acquire(&irq_lock);
  pic_irq_set_mask(irq);
  irq_callbacks[irq] = NULL;
  spinlock_release(&irq_lock);
}

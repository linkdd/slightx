#include <kernel/boot/idt.h>
#include <kernel/boot/int.h>
#include <kernel/boot/irq.h>
#include <kernel/boot/lapic.h>


typedef void (*isr_handler_fn)(interrupt_frame*);


static isr_handler_fn isr_table[256] = {0};


void isr_init(void) {
  for (usize i = 0; i < IDT_NUM_IRQS; i++) {
    isr_table[IDT_NUM_EXCEPTIONS + i] = irq_handler;
  }

  isr_table[IDT_GATE_LAPIC_TIMER]    = lapic_timer_handler;
  isr_table[IDT_GATE_LAPIC_SPURIOUS] = lapic_spurious_handler;
}


void isr_handler(interrupt_frame *iframe) {
  isr_handler_fn fn = isr_table[(u8)iframe->interrupt];
  if (fn != NULL) {
    fn(iframe);
  }
}

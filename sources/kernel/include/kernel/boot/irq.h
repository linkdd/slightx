#pragma once

#include <kernel/boot/int.h>


void irq_init(void);

void irq_handler(interrupt_frame *iframe);

interrupt_cb irq_get_callback  (u8 irq);
void         irq_set_callback  (u8 irq, interrupt_cb cb);
void         irq_clear_callback(u8 irq);

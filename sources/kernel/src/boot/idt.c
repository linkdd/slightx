#include <kernel/boot/gdt.h>
#include <kernel/boot/idt.h>
#include <kernel/boot/exc.h>

#include <klibc/mem/bytes.h>


extern u64 exc_stub_table[IDT_NUM_EXCEPTIONS];
extern u64 irq_stub_table[IDT_NUM_IRQS];


static idt_gate idt_gates[IDT_NUM_GATES];
static idt_ptr  idt_desc = {
  .limit = sizeof(idt_gate) * IDT_NUM_GATES - 1,
  .base  = (u64) &idt_gates
};


void idt_load(void) {
  memset(&idt_gates, 0, sizeof(idt_gate) * IDT_NUM_GATES);

  for (usize idx = 0; idx < IDT_NUM_EXCEPTIONS; idx++) {
    u8 ist = ((idx == EXC_DOUBLE_FAULT || idx == EXC_PAGE_FAULT || idx == EXC_NON_MASKABLE_INTERRUPT)
      ? 0x01
      : 0x00
    );
    idt_set_gate(idx, exc_stub_table[idx], 0x8E, ist);
  }

  for (usize idx = 0; idx < IDT_NUM_IRQS; idx++) {
    idt_set_gate(idx + IDT_NUM_EXCEPTIONS, irq_stub_table[idx], 0x8E, 0x0);
  }

  __asm__ volatile("lidt %0" :: "m"(idt_desc));
}


void idt_set_gate(usize idx, u64 base, u8 flags, u8 ist) {
  idt_gates[idx].offset_low  = (u16)  (base & 0x000000000000FFFF);
  idt_gates[idx].offset_mid  = (u16) ((base & 0x00000000FFFF0000) >> 16);
  idt_gates[idx].offset_high = (u32) ((base & 0xFFFFFFFF00000000) >> 32);
  idt_gates[idx].ist         = ist & 0x07;
  idt_gates[idx].selector    = kernel_code_segment;
  idt_gates[idx].flags       = flags;
  idt_gates[idx].zero        = 0;
}

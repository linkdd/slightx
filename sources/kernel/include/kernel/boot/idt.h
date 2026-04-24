#pragma once

#include <klibc/types.h>


#define IDT_NUM_GATES       256
#define IDT_NUM_EXCEPTIONS  32
#define IDT_NUM_IRQS        16


typedef struct idt_ptr idt_ptr;
struct idt_ptr {
  u16 limit;
  u64 base;
} __attribute__((packed));

typedef struct idt_gate idt_gate;
struct idt_gate {
  u16 offset_low;
  u16 selector;
  u8  ist;
  u8  flags;
  u16 offset_mid;
  u32 offset_high;
  // Reserved
  u32 zero;
} __attribute__((packed));


void idt_init(void);
void idt_load(void);

void idt_set_gate(usize idx, u64 base, u8 flags, u8 ist);

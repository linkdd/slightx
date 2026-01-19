#pragma once

#include <klibc/types.h>

#include <kernel/mem/vmm.h>
#include <kernel/boot/tss.h>


#define ISR_STACK_SIZE  0x4000  // 16 KB


typedef void (*cpu_fn)(void);

typedef struct percpu_data percpu_data;
struct percpu_data {
  usize  processor_id;
  u32    lapic_id;
  bool   is_bootstrap;

  cpu_fn current_fn;

  tss tss;

  alignas(16) u8 isr_stack[ISR_STACK_SIZE];
};


uptr percpu_get_segment(void);
void percpu_set_segment(uptr segment);

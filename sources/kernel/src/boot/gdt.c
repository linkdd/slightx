#include <kernel/boot/gdt.h>

#include <klibc/sync/lock.h>
#include <klibc/mem/bytes.h>


static gdt_gate gdt_descriptors[] = {
  // NULL Segment
  {},
  // Limine Protocol: 16-bit code descriptor
  {
    .limit_low  = 0xFFFF,
    .base_low   = 0,
    .base_mid   = 0,
    .access     = PRESENT | DPL(0) | NOT_SYS | EXEC | RW,
    .limit_high = 0,
    .flags      = 0,
    .base_high  = 0
  },
  // Limine Protocol: 16 bit data descriptor
  {
    .limit_low  = 0xFFFF,
    .base_low   = 0,
    .base_mid   = 0,
    .access     = PRESENT | DPL(0) | NOT_SYS | RW,
    .limit_high = 0,
    .flags      = 0,
    .base_high  = 0
  },
  // Limine Protocol: 32 bit code descriptor
  {
    .limit_low  = 0xFFFF,
    .base_low   = 0,
    .base_mid   = 0,
    .access     = PRESENT | DPL(0) | NOT_SYS | EXEC | RW,
    .limit_high = 0xF,
    .flags      = GRANULARITY_4K | SIZE_32,
    .base_high  = 0
  },
  // Limine Protocol: 32 bit data descriptor
  {
    .limit_low  = 0xFFFF,
    .base_low   = 0,
    .base_mid   = 0,
    .access     = PRESENT | DPL(0) | NOT_SYS | RW,
    .limit_high = 0xF,
    .flags      = GRANULARITY_4K | SIZE_32,
    .base_high  = 0
  },
  // Limine Protocol: 64 bit code descriptor
  {
    .limit_low  = 0,
    .base_low   = 0,
    .base_mid   = 0,
    .access     = PRESENT | DPL(0) | NOT_SYS | EXEC | RW,
    .limit_high = 0,
    .flags      = LONG_MODE,
    .base_high  = 0
  },
  // Limine Protocol: 64 bit data descriptor
  {
    .limit_low  = 0,
    .base_low   = 0,
    .base_mid   = 0,
    .access     = PRESENT | DPL(0) | NOT_SYS | RW,
    .limit_high = 0,
    .flags      = LONG_MODE,
    .base_high  = 0
  },
  // User Data Segment
  {
    .limit_low  = 0,
    .base_low   = 0,
    .base_mid   = 0,
    .access     = PRESENT | DPL(3) | NOT_SYS | RW,
    .limit_high = 0,
    .flags      = LONG_MODE | GRANULARITY_4K,
    .base_high  = 0
  },
  // User Code Segment
  {
    .limit_low  = 0,
    .base_low   = 0,
    .base_mid   = 0,
    .access     = PRESENT | DPL(3) | NOT_SYS | EXEC | RW,
    .limit_high = 0,
    .flags      = LONG_MODE | GRANULARITY_4K,
    .base_high  = 0
  },
  // Task State Segment
  {}, // Low
  {}, // High
};

static gdt_ptr gdt_pointer = {
  .limit = sizeof(gdt_descriptors) - 1,
  .base  = (u64) &gdt_descriptors
};


u16 kernel_code_segment = GDT_SEGMENT(GDT_SEG_IDX_KCODE);
u16 kernel_data_segment = GDT_SEGMENT(GDT_SEG_IDX_KDATA);
u16 user_code_segment   = GDT_SEGMENT(GDT_SEG_IDX_UCODE);
u16 user_data_segment   = GDT_SEGMENT(GDT_SEG_IDX_UDATA);
u16 tss_segment         = GDT_SEGMENT(GDT_SEG_IDX_TSS);

static spinlock gdt_lock = {};


void gdt_init(void) {
  spinlock_init(&gdt_lock);
}


void gdt_load(void) {
  asm volatile ("lgdt %0" : : "m"(gdt_pointer) : "memory");
  // No need to reload segments, the first 6 entries are the same
  // as the GDT setup by Limine, we merely added user segments.
}


void gdt_set_gate(usize segment, const gdt_gate *gate) {
  spinlock_acquire(&gdt_lock);
  memcpy(&gdt_descriptors[segment], gate, sizeof(gdt_gate));
  spinlock_release(&gdt_lock);
}

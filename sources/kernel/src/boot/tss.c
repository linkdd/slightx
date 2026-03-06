#include <kernel/boot/gdt.h>
#include <kernel/boot/tss.h>

#include <kernel/cpu/percpu.h>
#include <kernel/cpu/mp.h>


typedef struct tss_desc tss_desc;
struct tss_desc {
  u16 limit_low;
  u16 base_low;
  u8  base_mid;
  u8  access;
  u8  limit_high : 4;
  u8  flags      : 4;
  u8  base_high;
  u32 base_upper;
  u32 reserved;
} __attribute__((packed));

typedef union tss_gate tss_gate;
union tss_gate {
  gdt_gate entries[2];
  tss_desc desc;
};


void tss_load(void) {
  percpu_data *cpu_data = mp_get_percpu_data();

  u64 base  = (u64)&cpu_data->tss;
  u32 limit = sizeof(tss) - 1;

  tss_gate tss_descriptor = { .desc = {
    .limit_low  = limit & 0xFFFF,
    .base_low   = base  & 0xFFFF,
    .base_mid   = (base >> 16) & 0xFF,
    .access     = PRESENT | 0x09,
    .limit_high = (limit >> 16) & 0x0F,
    .flags      = 0,
    .base_high  = (base >> 24) & 0xFF,
    .base_upper = (base >> 32) & 0xFFFFFFFF,
    .reserved   = 0
  }};

  gdt_set_gate(GDT_SEG_IDX_TSS + 0, &tss_descriptor.entries[0]);
  gdt_set_gate(GDT_SEG_IDX_TSS + 1, &tss_descriptor.entries[1]);

  __asm__ volatile("ltr %0" : : "r"(tss_segment) : "memory");
}

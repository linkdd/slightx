#pragma once

#include <klibc/types.h>


// Access Bits
#define ACCESSED          (1 << 0)
#define RW                (1 << 1)
#define DC                (1 << 2)
#define EXEC              (1 << 3)
#define NOT_SYS           (1 << 4)
#define DPL(level)        (level << 5)
#define PRESENT           (1 << 7)

// Flags
#define LONG_MODE         (1 << 1)
#define SIZE_32           (1 << 2)
#define GRANULARITY_4K    (1 << 3)

// Segments
#define GDT_SEGMENT(idx)  ((idx) << 3)
#define GDT_SEG_IDX_KCODE  5
#define GDT_SEG_IDX_KDATA  6
#define GDT_SEG_IDX_UDATA  7
#define GDT_SEG_IDX_UCODE  8
#define GDT_SEG_IDX_TSS    9

// Privilege levels
#define RING0              0b00
#define RING1              0b01
#define RING2              0b10
#define RING3              0b11


extern u16 kernel_code_segment;
extern u16 kernel_data_segment;
extern u16 user_code_segment;
extern u16 user_data_segment;
extern u16 tss_segment;


typedef struct gdt_ptr gdt_ptr;
struct gdt_ptr {
  u16 limit;
  u64 base;
} __attribute__((packed));

typedef struct gdt_gate gdt_gate;
struct gdt_gate {
  u16 limit_low;
  u16 base_low;
  u8  base_mid;
  u8  access;
  u8  limit_high : 4;
  u8  flags      : 4;
  u8  base_high;
} __attribute__((packed));


void gdt_init(void);

void gdt_load(void);
void gdt_set_gate(usize segment, const gdt_gate *gate);

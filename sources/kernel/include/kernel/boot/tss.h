#pragma once

#include <klibc/types.h>

#include <kernel/boot/gdt.h>


typedef struct tss tss;
struct tss {
  u32 reserved0;
  u64 rsp0;
  u64 rsp1;
  u64 rsp2;
  u64 reserved1;
  u64 ist1;
  u64 ist2;
  u64 ist3;
  u64 ist4;
  u64 ist5;
  u64 ist6;
  u64 ist7;
  u32 reserved2;
  u32 reserved3;
} __attribute__((packed));


void tss_load(void);

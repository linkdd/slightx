#include <kernel/cpu/msr.h>


u64 rdmsr(u64 msr) {
  u32 low, high;
  asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((u64)high << 32) | low;
}


void wrmsr(u64 msr, u64 value) {
  u32 low  = value & 0xFFFFFFFF;
  u32 high = value >> 32;
  asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

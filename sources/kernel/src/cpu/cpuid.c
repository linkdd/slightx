#include <kernel/cpu/cpuid.h>


bool cpuid(u32 leaf, u32 subleaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx) {
  u32 cpuid_max;

  asm volatile (
    "cpuid"
    : "=a" (cpuid_max)
    : "a" (leaf & 0x80000000)
    : "ebx", "ecx", "edx"
  );

  if (leaf > cpuid_max) {
    return false;
  }

  asm volatile (
    "cpuid"
    : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
    : "a" (leaf), "c" (subleaf)
  );

  return true;
}

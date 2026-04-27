#include <kernel/cpu/cpuid.h>


static u32 cpuid__range_base(u32 leaf) {
  if      (leaf >= 0x80000000) return 0x80000000;
  else if (leaf >= 0x40000000) return 0x40000000;
  else                         return 0x00000000;
}


bool cpuid(u32 leaf, u32 subleaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx) {
  u32 base = cpuid__range_base(leaf);
  u32 max_leaf;

  __asm__ volatile(
    "cpuid"
    : "=a" (max_leaf)
    : "a" (base), "c" (0)
    : "ebx", "edx"
  );

  if (leaf > max_leaf) {
    return false;
  }

  __asm__ volatile(
    "cpuid"
    : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
    : "a" (leaf), "c" (subleaf)
  );

  return true;
}

#include <slightx/sys/mem.h>

#include "def.h"


void *sys_mmap(void *addr, usize length, sys_mmap_flags flags) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_MMAP),
      "D" ((u64)addr),
      "S" ((u64)length),
      "d" ((u64)flags)
    : "rcx", "r11", "memory"
  );

  void *result;
  __asm__ volatile (
    "mov %%rax, %0"
    : "=r" (result)
  );

  return result;
}


void sys_munmap(void *addr, usize length) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_MUNMAP),
      "D" ((u64)addr),
      "S" ((u64)length)
    : "rcx", "r11", "memory"
  );
}

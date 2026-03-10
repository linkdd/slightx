#include <slightx/sys/io.h>

#include "def.h"


i64 sys_puts(str s) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_PUTS),
      "D" ((u64)s.data),
      "S" ((u64)s.length)
    : "rcx", "r11", "memory"
  );

  i64 result;
  __asm__ volatile (
    "mov %%rax, %0"
    : "=r" (result)
  );

  return result;
}
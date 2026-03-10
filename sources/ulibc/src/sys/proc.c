#include <slightx/sys/proc.h>

#include "def.h"


[[noreturn]] void sys_exit(i32 code) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_EXIT),
      "D" ((u64)code)
    : "rcx", "r11", "memory"
  );

  __asm__ volatile ("ud2");
  unreachable();
}

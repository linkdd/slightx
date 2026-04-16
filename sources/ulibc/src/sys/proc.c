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


u32 sys_spawn(const_span binary, const task_startup_info *info) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_SPAWN),
      "D" ((u64)binary.data),
      "S" ((u64)binary.size),
      "d" ((u64)info)
    : "rcx", "r11", "memory"
  );

  u64 tid;
  __asm__ volatile (
    "mov %%rax, %0"
    : "=r" (tid)
  );

  return (u32)tid;
}


void sys_join(u32 tid) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_JOIN),
      "D" ((u64)tid)
    : "rcx", "r11", "memory"
  );
}

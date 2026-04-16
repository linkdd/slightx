#include <slightx/sys/cap.h>

#include "def.h"


i64 sys_send(cap_id cap, const_span msg) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_SEND),
      "D" ((u64)cap),
      "S" ((u64)msg.data),
      "d" ((u64)msg.size)
    : "rcx", "r11", "memory"
  );

  i64 result;
  __asm__ volatile (
    "mov %%rax, %0"
    : "=r" (result)
  );

  return result;
}


i64 sys_call(cap_id cap, const_span req, span resp) {
  register u64 r10 __asm__("r10") = (u64)resp.data;
  register u64 r8  __asm__("r8")  = (u64)resp.size;

  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_CALL),
      "D" ((u64)cap),
      "S" ((u64)req.data),
      "d" ((u64)req.size),
      "r" (r10),
      "r" (r8)
    : "rcx", "r11", "memory"
  );

  i64 result;
  __asm__ volatile (
    "mov %%rax, %0"
    : "=r" (result)
  );

  return result;
}


i64 sys_capctl(cap_id cap, u64 cmd, uptr arg) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_CAPCTL),
      "D" ((u64)cap),
      "S" ((u64)cmd),
      "d" ((u64)arg)
    : "rcx", "r11", "memory"
  );

  i64 result;
  __asm__ volatile (
    "mov %%rax, %0"
    : "=r" (result)
  );

  return result;
}

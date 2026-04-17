#include <slightx/sys/cap.h>

#include "def.h"


i64 sys_capread(cap_id cap, span msg) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_CAPREAD),
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


i64 sys_capwrite(cap_id cap, const_span msg) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_CAPWRITE),
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


i64 sys_capinvoke(cap_id cap, const_span req, span resp) {
  register u64 r10 __asm__("r10") = (u64)resp.data;
  register u64 r8  __asm__("r8")  = (u64)resp.size;

  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_CAPINVOKE),
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


i64 sys_capmap(cap_id cap, void *addr, usize size, u64 flags, void **mapped_addr_ptr) {
  register u64 r10 __asm__("r10") = (u64)flags;
  register u64 r8  __asm__("r8")  = (u64)mapped_addr_ptr;

  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_CAPMAP),
      "D" ((u64)cap),
      "S" ((u64)addr),
      "d" ((u64)size),
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


i64 sys_caprelease(cap_id cap) {
  __asm__ volatile (
    "syscall"
    :
    : "a" ((u64)SYSC_CAPRELEASE),
      "D" ((u64)cap)
    : "rcx", "r11", "memory"
  );

  i64 result;
  __asm__ volatile (
    "mov %%rax, %0"
    : "=r" (result)
  );

  return result;
}

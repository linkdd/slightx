#pragma once

#include <klibc/types.h>


#define SYS_EXIT   0
#define SYS_WRITE  1

#define SYS_MAX    2


// Syscall ABI (System V AMD64):
//   rax = syscall number
//   rdi = arg0
//   rsi = arg1
//   rdx = arg2
//   r10 = arg3  (rcx is clobbered by syscall instruction)
//   r8  = arg4
//   r9  = arg5
//
// Return value in rax.

typedef struct syscall_frame syscall_frame;
struct syscall_frame {
  u64 rax;  // syscall number / return value
  u64 rdi;
  u64 rsi;
  u64 rdx;
  u64 r10;
  u64 r8;
  u64 r9;
  u64 rcx;  // saved user RIP (set by syscall instruction)
  u64 r11;  // saved user RFLAGS (set by syscall instruction)
  u64 rbx;
  u64 rbp;
  u64 r12;
  u64 r13;
  u64 r14;
  u64 r15;
  u64 rsp;  // saved user RSP
};

typedef i64 (*syscall_fn)(syscall_frame *frame);


void syscall_init(void);
void syscall_load(void);

void syscall_handler(syscall_frame *frame);

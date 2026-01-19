#pragma once

#include <klibc/types.h>


typedef struct interrupt_frame interrupt_frame;
struct interrupt_frame {
  u64 cr4, cr3, cr2, cr0;
  u64 r15, r14, r13, r12, r11, r10, r9, r8;
  u64 rdi, rsi, rdx, rcx, rbx, rax, rbp;
  u64 interrupt;
  u64 err_code;
  u64 rip, cs, rflags, rsp, dss;
} __attribute__((packed));


typedef enum {
  // The generic handler should halt the CPU
  INTERRUPT_CONTROLFLOW_HALT = 0,
  // The generic handler should return
  INTERRUPT_CONTROLFLOW_RETURN,
} interrupt_controlflow;


typedef void (*interrupt_cb)(interrupt_frame*, interrupt_controlflow*);

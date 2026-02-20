[section .text]
[bits 64]

global syscall_entry_stub
extern syscall_handler

%define PERCPU_SYSCALL_KERNEL_RSP  0
%define PERCPU_SYSCALL_USER_RSP    8


syscall_entry_stub:
  swapgs

  mov [gs:PERCPU_SYSCALL_USER_RSP], rsp
  mov rsp, [gs:PERCPU_SYSCALL_KERNEL_RSP]

  push qword [gs:PERCPU_SYSCALL_USER_RSP]  ; rsp (user)
  push r15
  push r14
  push r13
  push r12
  push rbp
  push rbx
  push r11            ; user RFLAGS
  push rcx            ; user RIP
  push r9
  push r8
  push r10
  push rdx
  push rsi
  push rdi
  push rax            ; syscall number

  sti

  mov rdi, rsp
  call syscall_handler

  cli

  pop rax             ; return value
  pop rdi
  pop rsi
  pop rdx
  pop r10
  pop r8
  pop r9
  pop rcx             ; user RIP
  pop r11             ; user RFLAGS
  pop rbx
  pop rbp
  pop r12
  pop r13
  pop r14
  pop r15
  pop rsp             ; user RSP

  swapgs
  o64 sysret

[section .text]
[bits 64]

global task_kernelmode_trampoline
global task_usermode_trampoline
extern task_exit


task_kernelmode_trampoline:
  pop rax           ; fn
  pop rdi           ; arg
  call rax

  xor edi, edi
  call task_exit

  ud2 ; unreachable


task_usermode_trampoline:
  pop r15           ; arg
  pop rax           ; user_rip
  pop rbx           ; user_rsp
  pop rcx           ; user_cs
  pop rdx           ; user_ss

  push rdx          ; SS
  push rbx          ; RSP
  push 0x202        ; RFLAGS (IF=1)
  push rcx          ; CS
  push rax          ; RIP

  xor rax, rax
  xor rbx, rbx
  xor rcx, rcx
  xor rdx, rdx
  xor rsi, rsi
  mov rdi, r15      ; arg (System V ABI first argument)
  xor rbp, rbp
  xor r8, r8
  xor r9, r9
  xor r10, r10
  xor r11, r11
  xor r12, r12
  xor r13, r13
  xor r14, r14
  xor r15, r15

  swapgs

  iretq

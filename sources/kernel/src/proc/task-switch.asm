[section .text]
[bits 64]

global task_context_switch


task_context_switch:
.save_old_context:
  mov [rdi +  0], rsp
  mov [rdi +  8], rbp
  mov [rdi + 16], rbx
  mov [rdi + 24], r12
  mov [rdi + 32], r13
  mov [rdi + 40], r14
  mov [rdi + 48], r15

  mov rax, [rsp]
  mov [rdi + 56], rax

  pushfq
  pop rax
  mov [rdi + 64], rax

.load_new_context:
  mov rsp, [rsi +  0]
  mov rbp, [rsi +  8]
  mov rbx, [rsi + 16]
  mov r12, [rsi + 24]
  mov r13, [rsi + 32]
  mov r14, [rsi + 40]
  mov r15, [rsi + 48]

  mov rax, [rsi + 64]
  push rax
  popfq

  ret

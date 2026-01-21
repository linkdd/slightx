[section .text]
[bits 64]

global task_entrypoint_trampoline
extern thread_exit_from_task


task_entrypoint_trampoline:
  pop rax
  pop rdi
  call rax

  xor edi, edi
  call thread_exit_from_task

  ud2 ; unreachable

[section .text]
[bits 64]

%define kernel_data_segment   0x10

extern exc_handler
extern isr_handler

global exc_stub_table
global irq_stub_table

global lapic_timer_stub
global lapic_panic_stub
global lapic_spurious_stub
global lapic_tlb_flush_stub

%macro pushagrd 0
  push rax
  push rbx
  push rcx
  push rdx
  push rsi
  push rdi
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
%endmacro

%macro popagrd 0
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
%endmacro

%macro pushacrd 0
  mov rax, cr0
  push rax
  mov rax, cr2
  push rax
  mov rax, cr3
  push rax
  mov rax, cr4
  push rax
%endmacro

%macro popacrd 0
  pop rax
  mov cr4, rax
  pop rax
  mov cr3, rax
  pop rax
  mov cr2, rax
  pop rax
  mov cr0, rax
%endmacro

%macro exc_err_stub 1
exc_stub_%+%1:
  push qword %1
  jmp exc_common_stub
%endmacro

%macro exc_no_err_stub 1
exc_stub_%+%1:
  push qword 0
  push qword %1
  jmp exc_common_stub
%endmacro

%macro irq_stub 2
irq_stub_%+%1:
  push qword 0
  push qword %2
  jmp isr_common_stub
%endmacro

; Swap GS bases if interrupted from user mode (CPL=3).
%macro swapgs_if_usermode_entry 0
  test qword [rsp + 0x18], 3
  jz %%no_swapgs
  swapgs
%%no_swapgs:
%endmacro

%macro swapgs_if_usermode_exit 0
  test qword [rsp + 0x08], 3
  jz %%no_swapgs
  swapgs
%%no_swapgs:
%endmacro

%macro common_stub 2
%1:
  swapgs_if_usermode_entry
  push rbp
  mov rbp, rsp
  pushagrd
  pushacrd

  mov ax, ds
  push rax
  push qword 0

  mov ax, kernel_data_segment
  mov ds, ax
  mov es, ax

  lea rdi, [rsp + 0x10]
  call %2

  pop rax
  pop rax
  mov ds, ax
  mov es, ax

  popacrd
  popagrd
  pop rbp
  add rsp, 0x10
  swapgs_if_usermode_exit
  iretq
%endmacro

exc_no_err_stub 0
exc_no_err_stub 1
exc_no_err_stub 2
exc_no_err_stub 3
exc_no_err_stub 4
exc_no_err_stub 5
exc_no_err_stub 6
exc_no_err_stub 7
exc_err_stub    8
exc_no_err_stub 9
exc_err_stub    10
exc_err_stub    11
exc_err_stub    12
exc_err_stub    13
exc_err_stub    14
exc_no_err_stub 15
exc_no_err_stub 16
exc_err_stub    17
exc_no_err_stub 18
exc_no_err_stub 19
exc_no_err_stub 20
exc_err_stub    21
exc_no_err_stub 22
exc_no_err_stub 23
exc_no_err_stub 24
exc_no_err_stub 25
exc_no_err_stub 26
exc_no_err_stub 27
exc_no_err_stub 28
exc_err_stub    29
exc_err_stub    30
exc_no_err_stub 31

irq_stub  0, 32
irq_stub  1, 33
irq_stub  2, 34
irq_stub  3, 35
irq_stub  4, 36
irq_stub  5, 37
irq_stub  6, 38
irq_stub  7, 39
irq_stub  8, 40
irq_stub  9, 41
irq_stub 10, 42
irq_stub 11, 43
irq_stub 12, 44
irq_stub 13, 45
irq_stub 14, 46
irq_stub 15, 47

common_stub exc_common_stub, exc_handler
common_stub isr_common_stub, isr_handler

lapic_timer_stub:
  push qword 0
  push qword 0xE0
  jmp isr_common_stub

lapic_panic_stub:
  push qword 0
  push qword 0xFE
  jmp isr_common_stub

lapic_tlb_flush_stub:
  push qword 0
  push qword 0xFD
  jmp isr_common_stub

lapic_spurious_stub:
  push qword 0
  push qword 0xFF
  jmp isr_common_stub

align 8
exc_stub_table:
%assign i 0
%rep    32
  dq exc_stub_%+i
%assign i i+1
%endrep

align 8
irq_stub_table:
%assign i 0
%rep 16
  dq irq_stub_%+i
%assign i i+1
%endrep

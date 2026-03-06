; Minimal user-mode test program.
; Assembled as a flat binary, loaded from initrd at runtime.

[bits 64]
[org 0x400000]

_start:
  ; SYS_WRITE(buf, len): rax=1, rdi=buf, rsi=len
  lea rdi, [rel .msg]
  mov rsi, .msg_end - .msg
  mov rax, 1          ; SYS_WRITE
  syscall

  ; SYS_EXIT(0): rax=0, rdi=exit_code
  xor edi, edi
  mov rax, 0          ; SYS_EXIT
  syscall

  ; Should not reach here
  ud2

.msg:
  db "Hello from userspace!", 10
.msg_end:

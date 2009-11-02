global _start
extern kmain

; Setting up the Multiboot header
MODULEALIGN equ 1<<0                  ; align loaded modules on page boundaries
MEMINFO     equ 1<<1                  ; provide memory map
FLAGS       equ MODULEALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC       equ 0x1BADB002            ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)      ; checksum required


section .text
align 4
MultiBootHeader:
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

; Reserve initial kernel stack space
STACKSIZE equ 0x4000 ; 16k

_start:
	mov esp, stack + STACKSIZE
	push eax
	push ebx
	call kmain
	cli

end:
	hlt
	jmp end

section .bss
align 4
stack:
	resb STACKSIZE

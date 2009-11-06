[BITS 32]
[global start]
[extern kmain]

; Setting up the Multiboot header
MULTIBOOT_PAGE_ALIGN    equ 1<<0
MULTIBOOT_MEMORY_INFO   equ 1<<1
MULTIBOOT_HEADER_MAGIC  equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS  equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM      equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

[section .text]

ALIGN 4
MultiBootHeader:
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM

start:
	lgdt [trickgdt]
	mov cx, 0x10
	mov ds, cx
	mov es, cx
	mov fs, cx
	mov gs, cx
	mov ss, cx

	jmp 0x08:load
load:
	mov esp, stack

	push eax
	push ebx
	call kmain

	jmp $
end:
	hlt
	jmp end

[global gdt_flush]
[extern gp]

gdt_flush:
	lgdt [gp]

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:flush2
flush2:
	ret

[section .setup]

trickgdt:
	dw gdt_end - gdt - 1
	dd gdt

gdt:
	dd 0, 0                                            ; Null gate
	db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0x40 ; CS = 0x08, Base = 0x40000000, Limit = 0xFFFFFFFF, Type = 0x9A, Granularity = 0xCF
	db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0x40 ; DS = 0x10, Base = 0x40000000, Limit = 0xFFFFFFFF, Type = 0x92, Granularity = 0xCF

gdt_end:

[section .bss]

resb 0x1000
stack:

%macro ISR_NOERRCODE 1
	[GLOBAL isr%1]
	isr%1:
		cli
		push byte 0
		push byte %1
		jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
	[GLOBAL isr%1]
	isr%1:
		cli
		push byte %1
		jmp isr_common_stub
%endmacro

[EXTERN isr_handler]

isr_common_stub:
	pusha

	mov ax, ds
	push eax

	; Load kernel data segment descriptor
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	call isr_handler

	; Restore original data segment descriptor
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa
	add esp, 8
	sti
	iret
;isr_common_stub

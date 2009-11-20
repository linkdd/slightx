%define BASE    0x100
%define SIZE    1

[BITS 16]
[ORG 0x0]

jmp _start

print:
	push ax
	push bx
.startprint:
	lodsb
	cmp al, 0
	jz .endprint
	mov ah, 0x8E
	mov bx, 0x07
	int 0x10
	jmp .startprint
.endprint:
	pop bx
	pop ax
	ret

_start:
	mov ax, 0x07C0
	mov ds, ax
	mov es, ax
	mov ax, 0x8000
	mov ss, ax
	mov sp, 0xF000

	mov [bootdrv], dl

	mov si, msg00
	call print

	xor ax, ax
	int 0x13

	push es
	mov ax, BASE
	mov es, ax
	mov bx, 0

	mov ah, 2
	mov al, SIZE
	mov ch, 0
	mov cl, 2
	mov dh, 0
	mov dl, [bootdrv]
	int 0x13
	pop es

	jmp dword BASE:0
	
end:
	jmp end

msg00:   db "SXLoader: Loading stage2...", 13, 10, 0
bootdrv: db 0

times 510-($-$$) db 144
dw 0xAA55

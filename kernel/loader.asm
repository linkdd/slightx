MBOOT_PAGE_ALIGN    equ 1<<0    ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1    ; Provide your kernel with memory info
MBOOT_HEADER_MAGIC  equ 0x1BADB002 ; Multiboot Magic value
; NOTE: We do not use MBOOT_AOUT_KLUDGE. It means that GRUB does not
; pass us a symbol table.
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

[BITS 32]

[GLOBAL mboot]
[EXTERN code]
[EXTERN bss]
[EXTERN end]

mboot:
    dd  MBOOT_HEADER_MAGIC      ; GRUB will search for this value on each
                                ; 4-byte boundary in your kernel file
    dd  MBOOT_HEADER_FLAGS      ; How GRUB should load your file / settings
    dd  MBOOT_CHECKSUM          ; To ensure that the above values are correct

    dd  mboot                   ; Location of this descriptor
    dd  code                    ; Start of kernel '.text' (code) section.
    dd  bss                     ; End of kernel '.data' section.
    dd  end                     ; End of kernel.
    dd  start                   ; Kernel entry point (initial EIP).

[GLOBAL start]
[EXTERN kmain]

start:
    push esp
    push eax
    push ebx

    cli
    call kmain
    jmp $


[GLOBAL gdt_flush]    ; Allows the C code to call gdt_flush().

gdt_flush:
    mov eax, [esp + 4]  ; Get the pointer to the GDT, passed as a parameter.
    lgdt [eax]        ; Load the new GDT pointer

    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax        ; Load all data segment selectors
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush   ; 0x08 is the offset to our code segment: Far jump!
.flush:
    ret

[GLOBAL idt_flush]    ; Allows the C code to call idt_flush().

idt_flush:
    mov eax, [esp + 4]  ; Get the pointer to the IDT, passed as a parameter.
    lidt [eax]        ; Load the IDT pointer.
    ret

[GLOBAL tss_flush]    ; Allows our C code to call tss_flush().

tss_flush:
    mov ax, 0x2B      ; Load the index of our TSS structure - The index is
                      ; 0x28, as it is the 5th selector and each is 8 bytes
                      ; long, but we set the bottom two bits (making 0x2B)
                      ; so that it has an RPL of 3, not zero.
    ltr ax            ; Load 0x2B into the task state register.
    ret

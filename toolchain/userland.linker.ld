OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS
{
  . = 0x400000;

  .text : ALIGN(16) {
    *(.text.boot)
    *(.text*)
  }

  .rodata : ALIGN(16) { *(.rodata*) }
  .data   : ALIGN(16) { *(.data*) }
  .bss    : ALIGN(16) {
    __bss_start = .;
    *(.bss*)
    *(COMMON)
    __bss_end = .;
  }
}

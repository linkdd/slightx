ENTRY(kmain)

PHDRS
{
  text    PT_LOAD   FLAGS((1 << 0) | (1 << 2)); /* Execute + Read */
  rodata  PT_LOAD   FLAGS((1 << 2));            /* Read only */
  data    PT_LOAD   FLAGS((1 << 1) | (1 << 2)); /* Write + Read */
}

SECTIONS
{
  . = 0xFFFFFFFF80000000;

  .text :
  {
    kernel_region_start = .;
    *(.text .text.*)
  } :text

  . += CONSTANT(MAXPAGESIZE);

  .rodata :
  {
    *(.rodata .rodata.*)
  } :rodata

  . += CONSTANT(MAXPAGESIZE);

  .data :
  {
    *(.data .data.*)
  } :data

  .bss :
  {
    *(COMMON)
    *(.bss .bss.*)
    kernel_region_end = .;
  } :data

  /DISCARD/ : {
    *(.eh_frame)
    *(.note .note.*)
  }
}

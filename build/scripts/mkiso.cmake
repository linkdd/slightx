set(ISO_PATH ${DISTDIR}/slightx.iso)

message("-- Generating dist/ folder")
execute_process(
  COMMAND mkdir -p ${DISTDIR}
  COMMAND_ERROR_IS_FATAL ANY
)

message("-- Generating ISO image")
execute_process(
  COMMAND xorriso
    -as mkisofs
    -b boot/limine-bios-cd.bin
    -no-emul-boot
    -boot-load-size 4
    -boot-info-table
    --efi-boot boot/limine-uefi-cd.bin
    -efi-boot-part
    --efi-boot-image
    --protective-msdos-label
    ${SYSROOT}/iso
    -o ${ISO_PATH}

  COMMAND_ERROR_IS_FATAL ANY
)

message("-- Installing bootloader on ISO image")
execute_process(
  COMMAND ${LIMINE_BIN} bios-install ${ISO_PATH}
  COMMAND_ERROR_IS_FATAL ANY
)

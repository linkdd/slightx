set(INITRD_PATH ${SYSROOT}/iso/boot/initrd.tar)

message("-- Generating initrd")
execute_process(
  COMMAND tar
          --format=ustar
          --owner=root:0
          --group=root:0
          --sort=name
          -cf ${INITRD_PATH}
          .
  WORKING_DIRECTORY ${SYSROOT}/initrd
  COMMAND_ERROR_IS_FATAL ANY
)

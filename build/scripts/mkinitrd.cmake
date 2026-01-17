set(INITRD_PATH ${SYSROOT}/iso/boot/initrd.tar.gz)

message("-- Generating initrd")
execute_process(
  COMMAND ${CMAKE_COMMAND} -E tar "cfz" ${INITRD_PATH} .
  WORKING_DIRECTORY ${SYSROOT}/initrd
  COMMAND_ERROR_IS_FATAL ANY
)

cmake_minimum_required(VERSION 3.22.0)
project(slightx-apps-dummy ASM_NASM)

set(DUMMY_ASM_SOURCE ${PROJECT_SOURCE_DIR}/dummy.asm)

set_property(SOURCE ${DUMMY_ASM_SOURCE} PROPERTY LANGUAGE ASM_NASM)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dummy.bin
  COMMAND ${CMAKE_ASM_NASM_COMPILER} -f bin -o ${CMAKE_CURRENT_BINARY_DIR}/dummy.bin ${DUMMY_ASM_SOURCE}
  DEPENDS ${DUMMY_ASM_SOURCE}
  COMMENT "Assembling dummy.asm -> dummy.bin (flat binary)"
)

add_custom_target(dummy-app ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dummy.bin
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/dummy.bin
  DESTINATION initrd/apps
)

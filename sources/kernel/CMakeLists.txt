cmake_minimum_required(VERSION 3.22.0)
project(slightx-kernel C)

set(KERNEL_INCLUDES "${PROJECT_SOURCE_DIR}/include" CACHE INTERNAL "")

file(GLOB_RECURSE KERNEL_C_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.c
)

file(GLOB_RECURSE KERNEL_ASM_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.asm
)

set(KERNEL_SOURCES
  ${KERNEL_C_SOURCES}
  ${KERNEL_ASM_SOURCES}
)

set_property(SOURCE ${KERNEL_ASM_SOURCES} PROPERTY LANGUAGE ASM_NASM)
set_property(SOURCE ${KERNEL_ASM_SOURCES} PROPERTY NASM_FORMAT elf64)

add_executable(${PROJECT_NAME} ${KERNEL_SOURCES})
add_executable(slightx::kernel ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME "kernel.elf"
)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:${KERNEL_C_FLAGS}>)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${KERNEL_DEFINITIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${KERNEL_LD_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC ${KERNEL_INCLUDES}
  PRIVATE ${LIMINE_INCLUDES}
  PRIVATE ${FLANTERM_INCLUDES}
  PRIVATE ${LIBALLOC_INCLUDES}
  PRIVATE ${KLIBC_INCLUDES}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  vendor::limine
  vendor::flanterm
  vendor::liballoc
  slightx::klibc
)

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION iso/boot
)

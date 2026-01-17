cmake_minimum_required(VERSION 3.22.0)
project(slightx-kernel C)

set(KERNEL_INCLUDES "${PROJECT_SOURCE_DIR}/include" CACHE INTERNAL "")

file(GLOB_RECURSE KERNEL_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.c
)

add_executable(${PROJECT_NAME} ${KERNEL_SOURCES})
add_executable(slightx::kernel ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME "kernel.elf"
)

target_compile_options(${PROJECT_NAME} PRIVATE ${KERNEL_C_FLAGS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${KERNEL_DEFINITIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${KERNEL_LD_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC ${KERNEL_INCLUDES}
  PRIVATE ${LIMINE_INCLUDES}
  PRIVATE ${FLANTERM_INCLUDES}
  PRIVATE ${KLIBC_INCLUDES}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  vendor::limine
  vendor::flanterm
  slightx::klibc
)

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION iso/boot
)

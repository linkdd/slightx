cmake_minimum_required(VERSION 3.22.0)
project(limine C)

set(LIMINE_INCLUDES "${PROJECT_SOURCE_DIR}/include" CACHE INTERNAL "")

file(GLOB_RECURSE LIMINE_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.c
)

file(GLOB_RECURSE LIMINE_HEADERS
  ${LIMINE_INCLUDES}/*.h
)

add_library(${PROJECT_NAME} STATIC ${LIMINE_SOURCES} ${LIMINE_HEADERS})
add_library(vendor::limine ALIAS ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE ${KERNEL_C_FLAGS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${KERNEL_DEFINITIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${KERNEL_LD_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC ${LIMINE_INCLUDES}
)

file(GLOB_RECURSE LIMINE_EFI_BINARIES
  ${PROJECT_SOURCE_DIR}/bin/*.EFI
)

file(GLOB_RECURSE LIMINE_BIOS_BINARIES
  ${PROJECT_SOURCE_DIR}/bin/*.bin
  ${PROJECT_SOURCE_DIR}/bin/*.sys
)

install(
  FILES ${LIMINE_EFI_BINARIES}
  DESTINATION iso/EFI/BOOT
)

install(
  FILES ${LIMINE_BIOS_BINARIES}
  DESTINATION iso/boot
)

add_executable(limine-deploy ${PROJECT_SOURCE_DIR}/bin/limine.c)

cmake_minimum_required(VERSION 3.22.0)
project(slightx-klibc C)

set(KLIBC_INCLUDES "${PROJECT_SOURCE_DIR}/include" CACHE INTERNAL "")

file(GLOB_RECURSE KLIBC_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.c
)

add_library(${PROJECT_NAME} STATIC ${KLIBC_SOURCES})
add_library(slightx::klibc ALIAS ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE ${KERNEL_C_FLAGS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${KERNEL_DEFINITIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${KERNEL_LD_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC ${KLIBC_INCLUDES}
)

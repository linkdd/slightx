cmake_minimum_required(VERSION 3.22.0)
project(flanterm C)

set(FLANTERM_INCLUDES "${PROJECT_SOURCE_DIR}/upstream/src" CACHE INTERNAL "")

file(GLOB_RECURSE FLANTERM_SOURCES
  ${PROJECT_SOURCE_DIR}/upstream/src/*.c
)

add_library(${PROJECT_NAME} STATIC ${FLANTERM_SOURCES})
add_library(vendor::flanterm ALIAS ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE ${KERNEL_C_FLAGS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${KERNEL_DEFINITIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${KERNEL_LD_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC ${FLANTERM_INCLUDES}
)

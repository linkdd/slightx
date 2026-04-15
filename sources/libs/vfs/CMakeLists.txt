cmake_minimum_required(VERSION 3.22.0)
project(slightx-lib-vfs C)

set(VFS_INCLUDES "${PROJECT_SOURCE_DIR}/include" CACHE INTERNAL "")

file(GLOB_RECURSE VFS_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.c
)

add_library(${PROJECT_NAME} STATIC ${VFS_SOURCES})
add_library(slightx::lib::vfs ALIAS ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE ${USERLAND_C_FLAGS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${USERLAND_DEFINITIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${USERLAND_LD_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC ${VFS_INCLUDES} ${ULIBC_INCLUDES}
)
target_link_libraries(${PROJECT_NAME} PUBLIC
  slightx::ulibc
)

cmake_minimum_required(VERSION 3.22.0)
project(slightx-ulibc C)

set(ULIBC_INCLUDES "${PROJECT_SOURCE_DIR}/include" CACHE INTERNAL "")

file(GLOB_RECURSE ULIBC_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.c
)

add_library(${PROJECT_NAME} STATIC ${ULIBC_SOURCES})
add_library(slightx::ulibc ALIAS ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE ${USERLAND_C_FLAGS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${USERLAND_DEFINITIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${USERLAND_LD_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC ${ULIBC_INCLUDES} ${LIBALLOC_INCLUDES}
)

target_link_libraries(${PROJECT_NAME} PUBLIC vendor::liballoc-user)


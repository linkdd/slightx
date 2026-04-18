cmake_minimum_required(VERSION 3.22.0)
project(liballoc C)

set(LIBALLOC_INCLUDES "${PROJECT_SOURCE_DIR}/include" CACHE INTERNAL "")

file(GLOB_RECURSE LIBALLOC_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.c
)

file(GLOB_RECURSE LIBALLOC_HEADERS
  ${LIBALLOC_INCLUDES}/*.h
)

add_library(${PROJECT_NAME} STATIC ${LIBALLOC_SOURCES} ${LIBALLOC_HEADERS})
add_library(vendor::liballoc ALIAS ${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE ${KERNEL_C_FLAGS} -Wno-sign-compare)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${KERNEL_DEFINITIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${KERNEL_LD_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC ${LIBALLOC_INCLUDES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")


add_library(${PROJECT_NAME}-user STATIC ${LIBALLOC_SOURCES} ${LIBALLOC_HEADERS})
add_library(vendor::liballoc-user ALIAS ${PROJECT_NAME}-user)

target_compile_options(${PROJECT_NAME}-user PRIVATE ${USERLAND_C_FLAGS} -Wno-sign-compare)
target_compile_definitions(${PROJECT_NAME}-user PRIVATE ${USERLAND_DEFINITIONS})
target_link_options(${PROJECT_NAME}-user PRIVATE ${USERLAND_LD_FLAGS})
target_include_directories(${PROJECT_NAME}-user
  PUBLIC ${LIBALLOC_INCLUDES}
)

set_target_properties(${PROJECT_NAME}-user PROPERTIES PREFIX "")

cmake_minimum_required(VERSION 3.22.0)
project(slightx-apps-dummy C)

file(GLOB_RECURSE APP_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.c
)

add_executable(${PROJECT_NAME} ${APP_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME "dummy.bin"
)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:${USERLAND_C_FLAGS}>)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${USERLAND_DEFINITIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${USERLAND_LD_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${ULIBC_INCLUDES}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
  slightx::ulibc
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/dummy.bin
  DESTINATION initrd/apps
)

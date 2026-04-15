cmake_minimum_required(VERSION 3.22.0)
project(slightx C ASM_NASM)

include("build/cmake/config.cmake")
include("build/cmake/build-deps.cmake")
include("build/cmake/compile-flags.cmake")
include("build/cmake/vars.cmake")

add_subdirectory(vendor)
add_subdirectory(sources/klibc)
add_subdirectory(sources/kernel)
add_subdirectory(sources/ulibc)
add_subdirectory(sources/apps)
add_subdirectory(config)

install(CODE "
  execute_process(
    COMMAND ${CMAKE_COMMAND}
      -DSYSROOT=${SYSROOT}
      -DDISTDIR=${DISTDIR}
      -DLIMINE_BIN=$<TARGET_FILE:limine-deploy>
      -P
      ${CMAKE_CURRENT_LIST_DIR}/build/scripts/mkiso.cmake
  )
")

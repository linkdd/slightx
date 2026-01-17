set(KERNEL_C_FLAGS
  -Wall
  -Wextra
  -ffreestanding
  -fno-pie
  -fno-pic
  -march=x86-64
  -mabi=sysv
  -mno-red-zone
  -mcmodel=kernel
  -mno-aes
  -mno-mmx
  -mno-pclmul
  -mno-sse
  -mno-sse2
  -mno-sse3
  -mno-sse4
  -mno-sse4a
  -mno-fma4
  -mno-ssse3
  -msoft-float
)

set(KERNEL_LD_FLAGS
  -static
  -nostdlib
  -Wl,-z,max-page-size=0x1000
  -Wl,-z,noexecstack
  -Wl,--build-id=none
  -Wl,-T,${CMAKE_SOURCE_DIR}/toolchain/linker.ld
)

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  set(KERNEL_C_FLAGS -g ${KERNEL_C_FLAGS})
endif()

#pragma once

#include <slightx/types.h>


typedef enum : u8 {
  MMAP_ACCESS_READ  = 1 << 0,
  MMAP_ACCESS_WRITE = 1 << 1,
  MMAP_ACCESS_EXEC  = 1 << 2,
  MMAP_FIXED        = 1 << 3,
} sys_mmap_flags;


void *sys_mmap  (void *addr, usize length, sys_mmap_flags flags);
void  sys_munmap(void *addr, usize length);

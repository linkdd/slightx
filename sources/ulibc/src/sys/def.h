#pragma once

#include <slightx/types.h>


typedef enum : u64 {
  SYSC_EXIT  = 0,
  SYSC_SPAWN,
  SYSC_JOIN,

  SYSC_MMAP,
  SYSC_MUNMAP,

  SYSC_PUTS,

  SYSC__COUNT,
} syscall_num;

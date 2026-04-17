#pragma once

#include <slightx/types.h>


typedef enum : u64 {
  SYSC_EXIT  = 0,
  SYSC_SPAWN,
  SYSC_JOIN,

  SYSC_MMAP,
  SYSC_MUNMAP,

  SYSC_CAPREAD,
  SYSC_CAPWRITE,
  SYSC_CAPINVOKE,
  SYSC_CAPMAP,
  SYSC_CAPCTL,
  SYSC_CAPRELEASE,

  SYSC__COUNT,
} syscall_num;

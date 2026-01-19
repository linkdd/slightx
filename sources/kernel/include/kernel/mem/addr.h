#pragma once

#include <klibc/types.h>


typedef union {
  uptr  addr;
  void *ptr;
} physical_address;

typedef union {
  uptr  addr;
  void *ptr;
} virtual_address;

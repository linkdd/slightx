#pragma once

#include <klibc/types.h>
#include <klibc/mem/alloc.h>

#include <kernel/proc/capabilities.h>


typedef struct console_cap console_cap;
struct console_cap {
  cap_obj base;

  allocator a;
};


cap_obj *make_console_cap(allocator a);

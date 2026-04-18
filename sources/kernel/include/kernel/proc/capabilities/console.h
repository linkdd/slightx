#pragma once

#include <klibc/types.h>

#include <kernel/proc/capabilities.h>


typedef struct console_cap console_cap;
struct console_cap {
  cap_obj base;
};


cap_obj *make_console_cap(allocator a);

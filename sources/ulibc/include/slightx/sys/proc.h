#pragma once

#include <slightx/types.h>
#include <slightx/mem/span.h>


[[noreturn]] void sys_exit(i32 code);

u32  sys_spawn(const_span binary);
void sys_join (u32 tid);

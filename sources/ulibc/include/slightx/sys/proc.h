#pragma once

#include <slightx/types.h>
#include <slightx/mem/span.h>
#include <slightx/mem/str.h>


typedef struct task_startup_info task_startup_info;
struct task_startup_info {
  strv  args;
  strv  envvars;
};


[[noreturn]] void sys_exit(i32 code);

u32  sys_spawn(const_span binary, const task_startup_info *info);
void sys_join (u32 tid);

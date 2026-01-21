#pragma once

#include <klibc/types.h>


u32 thread_current_id(void);

void thread_sleep(u64 ns);
void thread_join (u32 tid);

[[noreturn]] void thread_exit_from_task(i32 exit_code);

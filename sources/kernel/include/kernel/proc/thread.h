#pragma once

#include <klibc/types.h>

#include <kernel/proc/task.h>


tid thread_current_id(void);

void thread_sleep(u64 ns);
void thread_join (tid tid);

[[noreturn]] void thread_exit_from_task(i32 exit_code);

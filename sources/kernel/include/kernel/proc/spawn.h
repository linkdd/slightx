#pragma once

#include <klibc/types.h>
#include <klibc/mem/span.h>
#include <klibc/mem/str.h>

#include <kernel/proc/task.h>


#define USER_CODE_BASE  0x400000ULL


tid spawn_kernel_task(task_entrypoint entrypoint);
tid spawn_user_task  (const_span binary);

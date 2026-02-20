#pragma once

#include <klibc/types.h>
#include <klibc/mem/span.h>

#include <kernel/proc/task.h>


u32 spawn_kernel_task(task_entrypoint entrypoint);
u32 spawn_user_task  (const_span binary);

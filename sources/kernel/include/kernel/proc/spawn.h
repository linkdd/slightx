#pragma once

#include <klibc/types.h>
#include <klibc/mem/span.h>
#include <klibc/mem/str.h>

#include <kernel/proc/task.h>


RESULT_DECL(tid, str);

tid spawn_kernel_task(task_entrypoint entrypoint);
tid spawn_user_task  (const_span binary);

RESULT(tid, str) spawn_executable(str path);

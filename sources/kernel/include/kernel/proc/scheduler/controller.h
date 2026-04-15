#pragma once

#include <klibc/types.h>

#include <kernel/proc/scheduler/runqueue.h>
#include <kernel/proc/scheduler/sleeperlist.h>
#include <kernel/proc/task.h>


typedef struct scheduler scheduler;
struct scheduler {
  u64 uptime_ns;

  runqueue    tasks;
  runqueue    cleanup;
  sleeperlist sleepers;

  task *current;
  task *idle;
};

void scheduler_init(void);
void scheduler_load(void);

void scheduler_tick(u64 ns);

u32 scheduler_get_next_tid(void);

task *scheduler_get_current_task (void);
task *scheduler_get_task_by_id   (u32 tid);
void  scheduler_kill_current_task(i32 exit_code);

waiter scheduler_make_waiter (task *t);
void   scheduler_wakeup_after(u64 ns, waitqueue_item *wait);

void scheduler_cleanup(void);
void scheduler_yield  (void);

[[noreturn]] void scheduler_idle_loop(void);

#pragma once

#include <klibc/types.h>

#include <klibc/sync/lock.h>
#include <klibc/sync/waitqueue.h>
#include <klibc/chrono/timer.h>

#include <kernel/proc/task.h>


typedef struct runqueue runqueue;
struct runqueue {
  spinlock  lock;
  task     *head;
  task     *tail;
  usize     count;
};

typedef struct sleeper sleeper;
struct sleeper {
  timer timer;
  u64   duration_ns;

  waitqueue_item *w;

  struct {
    struct sleeper *prev;
    struct sleeper *next;
  } siblings;
};

typedef struct sleeperlist sleeperlist;
struct sleeperlist {
  spinlock  lock;
  sleeper  *head;
  sleeper  *tail;
  usize     count;
};

void runqueue_init(runqueue *self);

void  runqueue_enqueue(runqueue *self, task *t);
task *runqueue_dequeue(runqueue *self);
void  runqueue_remove (runqueue *self, task *t);

bool runqueue_is_empty(runqueue *self);


void sleeperlist_init(sleeperlist *self);
void sleeperlist_add (sleeperlist *self, u64 duration, waitqueue_item *w);
void sleeperlist_tick(sleeperlist *self);


void scheduler_init(void);
void scheduler_load(void);

u32 scheduler_get_next_tid(void);

task *scheduler_get_current_task (void);
task *scheduler_get_task_by_id   (u32 tid);
void  scheduler_kill_current_task(i32 exit_code);

waiter scheduler_make_waiter (task *t);
void   scheduler_wakeup_after(u64 ns, waitqueue_item *wait);

void scheduler_cleanup(void);
void scheduler_yield  (void);

[[noreturn]] void scheduler_idle_loop(void);

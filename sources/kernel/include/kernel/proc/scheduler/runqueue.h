#pragma once

#include <klibc/types.h>

#include <klibc/sync/lock.h>
#include <klibc/sync/waitqueue.h>

#include <kernel/proc/task.h>


typedef struct runqueue runqueue;
struct runqueue {
  spinlock  lock;
  task     *head;
  task     *tail;
  usize     count;
};

void runqueue_init(runqueue *self);

void  runqueue_enqueue(runqueue *self, task *t);
task *runqueue_dequeue(runqueue *self);
void  runqueue_remove (runqueue *self, task *t);

bool  runqueue_is_empty  (runqueue *self);
task *runqueue_find_by_id(runqueue *self, u32 tid);

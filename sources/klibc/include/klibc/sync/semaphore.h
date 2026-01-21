#pragma once

#include <klibc/types.h>

#include <klibc/sync/lock.h>
#include <klibc/sync/waitqueue.h>


typedef enum {
  SEM_ACQUIRED = 0,
  SEM_SHOULD_WAIT,
} semaphore_status;

typedef struct semaphore semaphore;
struct semaphore {
  spinlock  lock;
  isize     count;
  waitqueue wq;
};


void semaphore_init(semaphore *self, isize initial_count);

semaphore_status semaphore_acquire    (semaphore *self, waitqueue_item *wait);
bool             semaphore_try_acquire(semaphore *self);
void             semaphore_release    (semaphore *self);

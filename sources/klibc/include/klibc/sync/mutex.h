#pragma once

#include <klibc/types.h>

#include <klibc/sync/lock.h>
#include <klibc/sync/waitqueue.h>


typedef enum {
  MUTEX_ACQUIRED = 0,
  MUTEX_SHOULD_WAIT,
} mutex_status;

typedef struct mutex mutex;
struct mutex {
  spinlock   lock;
  void      *owner;
  waitqueue  wq;
};


void mutex_init(mutex *self);

mutex_status mutex_lock    (mutex *self, void *owner, waitqueue_item *wait);
bool         mutex_try_lock(mutex *self, void *owner);
void         mutex_unlock  (mutex *self, void *owner);

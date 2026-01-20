#pragma once

#include <klibc/types.h>

#include <klibc/sync/lock.h>
#include <klibc/sync/waitqueue.h>


typedef struct semaphore semaphore;
struct semaphore {
  spinlock  lock;
  isize     count;
  waitqueue wq;
};


void semaphore_init(semaphore *self, isize initial_count);

void semaphore_acquire    (semaphore *self, wq_handle handle, void (*block)(void *), void *udata);
bool semaphore_try_acquire(semaphore *self);
void semaphore_release    (semaphore *self);

isize semaphore_get_count(semaphore *self);

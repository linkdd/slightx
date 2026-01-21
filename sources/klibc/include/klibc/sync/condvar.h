#pragma once

#include <klibc/types.h>

#include <klibc/sync/lock.h>
#include <klibc/sync/waitqueue.h>


typedef struct condvar condvar;
struct condvar {
  spinlock  lock;
  waitqueue wq;
};


void condvar_init(condvar *self);

void condvar_wait     (condvar *self, waitqueue_item *wait);
void condvar_signal   (condvar *self);
void condvar_broadcast(condvar *self);

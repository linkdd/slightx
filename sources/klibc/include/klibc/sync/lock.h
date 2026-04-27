#pragma once

#include <klibc/types.h>


typedef struct spinlock spinlock;
struct spinlock {
  atomic_bool lock_val;
};

typedef struct syncpoint syncpoint;
struct syncpoint {
  spinlock lock;
  usize    remaining;
};


void pause(void);

void spinlock_init(spinlock *self);

void spinlock_acquire    (spinlock *self);
bool spinlock_try_acquire(spinlock *self);
void spinlock_release    (spinlock *self);


void syncpoint_init(syncpoint *self);

void syncpoint_set   (syncpoint *self, usize count);
void syncpoint_notify(syncpoint *self);
void syncpoint_wait  (syncpoint *self);

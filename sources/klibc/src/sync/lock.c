#include <klibc/sync/lock.h>
#include <klibc/assert.h>


static void pause(void) {
  __builtin_ia32_pause();
}


void spinlock_init(spinlock *self) {
  assert(self != NULL);

  atomic_store(&self->lock_val, false);
}


void spinlock_acquire(spinlock *self) {
  assert(self != NULL);

  while (true) {
    // Optimistically assume the lock is free on the first try
    if (!atomic_exchange_explicit(&self->lock_val, true, memory_order_acquire)) {
      return;
    }

    // Wait for lock to be released without generating cache misses
    while (atomic_load_explicit(&self->lock_val, memory_order_relaxed)) {
      // Issue PAUSE or YIELD instruction to reduce contention between
      // hyper-threads
      pause();
    }
  }
}


bool spinlock_try_acquire(spinlock *self) {
  assert(self != NULL);

  // First do a relaxed load to check if lock is free in order to prevent
  // unnecessary cache misses if someone does while(!try_lock())
  return (
    !atomic_load_explicit(&self->lock_val, memory_order_relaxed) &&
    !atomic_exchange_explicit(&self->lock_val, true, memory_order_acquire)
  );
}


void spinlock_release(spinlock *self) {
  assert(self != NULL);

  atomic_store_explicit(&self->lock_val, false, memory_order_release);
}


void syncpoint_init(syncpoint *self) {
  assert(self != NULL);

  spinlock_init(&self->lock);
  self->remaining = 0;
}


void syncpoint_set(syncpoint *self, usize count) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);
  self->remaining = count;
  spinlock_release(&self->lock);
}


void syncpoint_notify(syncpoint *self) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);
  if (self->remaining > 0) {
    self->remaining--;
  }
  spinlock_release(&self->lock);
}


void syncpoint_wait(syncpoint *self) {
  assert(self != NULL);

  while (true) {
    spinlock_acquire(&self->lock);
    size_t count = self->remaining;
    spinlock_release(&self->lock);

    if (count == 0) {
      return;
    }

    pause();
  }
}

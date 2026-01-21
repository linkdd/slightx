#include <klibc/sync/semaphore.h>
#include <klibc/assert.h>


void semaphore_init(semaphore *self, isize initial_count) {
  assert(self != NULL);

  spinlock_init (&self->lock);
  waitqueue_init(&self->wq);

  self->count = initial_count;
}


semaphore_status semaphore_acquire(semaphore *self, waitqueue_item *wait) {
  assert(self != NULL);
  assert(wait != NULL);

  spinlock_acquire(&self->lock);

  self->count--;

  if (self->count >= 0) {
    spinlock_release(&self->lock);
    return SEM_ACQUIRED;
  }
  else {
    waitqueue_add(&self->wq, wait);
    spinlock_release(&self->lock);
    return SEM_SHOULD_WAIT;
  }
}


bool semaphore_try_acquire(semaphore *self) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);

  if (self->count > 0) {
    self->count--;
    spinlock_release(&self->lock);
    return true;
  }
  else {
    spinlock_release(&self->lock);
    return false;
  }
}


void semaphore_release(semaphore *self) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);

  self->count++;

  if (self->count <= 0) {
    spinlock_release(&self->lock);
    waitqueue_wake_one(&self->wq);
  }
  else {
    spinlock_release(&self->lock);
  }
}

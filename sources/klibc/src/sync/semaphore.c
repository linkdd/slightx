#include <klibc/sync/semaphore.h>
#include <klibc/assert.h>


void semaphore_init(semaphore *self, isize initial_count) {
  assert(self != NULL);

  spinlock_init (&self->lock);
  waitqueue_init(&self->wq);

  self->count = initial_count;
}


void semaphore_acquire(
  semaphore *self,
  wq_handle  handle,

  void (*block)(void *), void *udata
) {
  assert(self != NULL);
  assert(block != NULL);

  wq_entry entry;
  waitqueue_entry_init(&entry, handle);

  spinlock_acquire(&self->lock);
  self->count--;

  if (self->count < 0) {
    waitqueue_add(&self->wq, &entry);
    spinlock_release(&self->lock);

    block(udata);
  }
  else {
    spinlock_release(&self->lock);
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


isize semaphore_get_count(semaphore *self) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);
  isize count = self->count;
  spinlock_release(&self->lock);

  return count;
}

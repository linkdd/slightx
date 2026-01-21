#include <klibc/sync/mutex.h>
#include <klibc/assert.h>


void mutex_init(mutex *self) {
  assert(self != NULL);

  spinlock_init (&self->lock);
  waitqueue_init(&self->wq);

  self->owner = NULL;
}


mutex_status mutex_lock(mutex *self, void *owner, waitqueue_item *wait) {
  assert(self != NULL);
  assert(owner != NULL);
  assert(wait != NULL);

  spinlock_acquire(&self->lock);

  if (self->owner == NULL) {
    self->owner = owner;
    spinlock_release(&self->lock);
    return MUTEX_ACQUIRED;
  }
  else {
    waitqueue_add(&self->wq, wait);
    spinlock_release(&self->lock);
    return MUTEX_SHOULD_WAIT;
  }
}


bool mutex_try_lock(mutex *self, void *owner) {
  assert(self != NULL);
  assert(owner != NULL);

  spinlock_acquire(&self->lock);

  if (self->owner == NULL) {
    self->owner = owner;
    spinlock_release(&self->lock);
    return true;
  }
  else {
    spinlock_release(&self->lock);
    return false;
  }
}


void mutex_unlock(mutex *self, void *owner) {
  assert(self != NULL);
  assert(owner != NULL);

  spinlock_acquire(&self->lock);
  self->owner = NULL;
  spinlock_release(&self->lock);

  waitqueue_wake_one(&self->wq);
}

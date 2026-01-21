#include <klibc/sync/condvar.h>
#include <klibc/assert.h>


void condvar_init(condvar *self) {
  assert(self != NULL);

  spinlock_init (&self->lock);
  waitqueue_init(&self->wq);
}


void condvar_wait(condvar *self, waitqueue_item *wait) {
  assert(self != NULL);
  assert(wait != NULL);

  spinlock_acquire(&self->lock);
  waitqueue_add   (&self->wq, wait);
  spinlock_release(&self->lock);
}


void condvar_signal(condvar *self) {
  assert(self != NULL);

  waitqueue_wake_one(&self->wq);
}


void condvar_broadcast(condvar *self) {
  assert(self != NULL);

  waitqueue_wake_all(&self->wq);
}

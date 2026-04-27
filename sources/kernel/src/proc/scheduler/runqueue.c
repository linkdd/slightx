#include <klibc/assert.h>

#include <kernel/proc/scheduler/runqueue.h>


void runqueue_init(runqueue *self) {
  assert(self != NULL);

  spinlock_init(&self->lock);
  self->head  = NULL;
  self->tail  = NULL;
  self->count = 0;
}


void runqueue_enqueue(runqueue *self, task *t) {
  assert(self != NULL);
  assert(t != NULL);

  spinlock_acquire(&self->lock);

  t->scheduling.siblings.prev = self->tail;
  t->scheduling.siblings.next = NULL;

  if (self->tail != NULL) {
    self->tail->scheduling.siblings.next = t;
  }
  self->tail = t;

  if (self->head == NULL) {
    self->head = t;
  }

  self->count += 1;

  spinlock_release(&self->lock);
}


task *runqueue_dequeue(runqueue *self) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);

  task *t = self->head;
  if (t != NULL) {
    self->head = t->scheduling.siblings.next;
    if (self->head != NULL) {
      self->head->scheduling.siblings.prev = NULL;
    }
    else {
      self->tail = NULL;
    }

    t->scheduling.siblings.prev = NULL;
    t->scheduling.siblings.next = NULL;

    self->count -= 1;
  }

  spinlock_release(&self->lock);

  return t;
}


void runqueue_remove(runqueue *self, task *t) {
  assert(self != NULL);
  assert(t != NULL);

  spinlock_acquire(&self->lock);

  if (t->scheduling.siblings.prev != NULL) {
    t->scheduling.siblings.prev->scheduling.siblings.next = t->scheduling.siblings.next;
  }
  else {
    self->head = t->scheduling.siblings.next;
  }

  if (t->scheduling.siblings.next != NULL) {
    t->scheduling.siblings.next->scheduling.siblings.prev = t->scheduling.siblings.prev;
  }
  else {
    self->tail = t->scheduling.siblings.prev;
  }

  t->scheduling.siblings.prev = NULL;
  t->scheduling.siblings.next = NULL;

  self->count -= 1;

  spinlock_release(&self->lock);
}


bool runqueue_is_empty(runqueue *self) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);
  bool is_empty = (self->count == 0);
  spinlock_release(&self->lock);

  return is_empty;
}


task *runqueue_find_by_id(runqueue *self, u32 tid) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);

  for (
    task *iter = self->head;
    iter != NULL;
    iter = iter->scheduling.siblings.next
  ) {
    if (iter->id == tid) {
      task_acquire(iter);
      spinlock_release(&self->lock);
      return iter;
    }
  }

  spinlock_release(&self->lock);
  return NULL;
}


#include <klibc/sync/waitqueue.h>
#include <klibc/assert.h>


void waitqueue_entry_init(wq_entry *self, wq_handle handle) {
  assert(self != NULL);

  self->handle = handle;

  self->siblings.prev = NULL;
  self->siblings.next = NULL;
}


void waitqueue_init(waitqueue *self) {
  assert(self != NULL);

  spinlock_init(&self->lock);
  self->head = NULL;
  self->tail = NULL;
}


void waitqueue_add(waitqueue *self, wq_entry *entry) {
  assert(self != NULL);
  assert(entry != NULL);
  assert(entry->siblings.prev == NULL);
  assert(entry->siblings.next == NULL);

  spinlock_acquire(&self->lock);

  if (self->tail == NULL) {
    self->head = entry;
    self->tail = entry;

    entry->siblings.prev = NULL;
    entry->siblings.next = NULL;
  }
  else {
    entry->siblings.prev = self->tail;
    entry->siblings.next = NULL;

    self->tail->siblings.next = entry;
    self->tail                = entry;
  }

  spinlock_release(&self->lock);
}


void waitqueue_del(waitqueue *self, wq_entry *entry) {
  assert(self != NULL);
  assert(entry != NULL);

  spinlock_acquire(&self->lock);

  if (
    entry->siblings.prev == NULL &&
    entry->siblings.next == NULL &&
    self->head != entry
  ) {
    spinlock_release(&self->lock);
    return;
  }

  if (entry->siblings.prev != NULL) {
    entry->siblings.prev->siblings.next = entry->siblings.next;
  }
  else {
    self->head = entry->siblings.next;
  }

  if (entry->siblings.next != NULL) {
    entry->siblings.next->siblings.prev = entry->siblings.prev;
  }
  else {
    self->tail = entry->siblings.prev;
  }

  entry->siblings.prev = NULL;
  entry->siblings.next = NULL;

  spinlock_release(&self->lock);
}


void waitqueue_wake_one(waitqueue *self) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);

  wq_entry *entry = self->head;
  if (entry != NULL) {
    self->head = entry->siblings.next;

    if (self->head != NULL) {
      self->head->siblings.prev = NULL;
    }
    else {
      self->tail = NULL;
    }

    entry->siblings.prev = NULL;
    entry->siblings.next = NULL;

    wq_handle handle = entry->handle;
    spinlock_release(&self->lock);

    if (handle.wake != NULL) {
      handle.wake(handle.udata);
    }
  }
  else {
    spinlock_release(&self->lock);
  }
}


void waitqueue_wake_all(waitqueue *self) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);

  wq_entry *entry = self->head;

  self->head = NULL;
  self->tail = NULL;

  spinlock_release(&self->lock);

  while (entry != NULL) {
    wq_entry *next = entry->siblings.next;

    entry->siblings.prev = NULL;
    entry->siblings.next = NULL;

    if (entry->handle.wake != NULL) {
      entry->handle.wake(entry->handle.udata);
    }

    entry = next;
  }
}


bool waitqueue_is_empty(waitqueue *self) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);
  bool empty = (self->head == NULL);
  spinlock_release(&self->lock);

  return empty;
}

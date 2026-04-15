#include <klibc/assert.h>

#include <kernel/proc/scheduler/sleeperlist.h>
#include <kernel/chrono/time.h>
#include <kernel/mem/heap.h>


void sleeperlist_init(sleeperlist *self) {
  assert(self != NULL);

  spinlock_init(&self->lock);
  self->head  = NULL;
  self->tail  = NULL;
  self->count = 0;
}


void sleeperlist_add(sleeperlist *self, u64 duration, waitqueue_item *w) {
  assert(self != NULL);
  assert(w != NULL);

  allocator a = heap_allocator();

  sleeper *s = allocate(a, sizeof(sleeper));
  timer_init (&s->timer, uptime_ns);
  timer_start(&s->timer);

  s->duration_ns = duration;
  s->w           = w;

  spinlock_acquire(&self->lock);

  s->siblings.prev = self->tail;
  s->siblings.next = NULL;

  if (self->tail != NULL) {
    self->tail->siblings.next = s;
  }
  self->tail = s;

  if (self->head == NULL) {
    self->head = s;
  }

  self->count += 1;

  spinlock_release(&self->lock);
}


void sleeperlist_tick(sleeperlist *self) {
  assert(self != NULL);

  allocator a = heap_allocator();

  spinlock_acquire(&self->lock);

  sleeper *s = self->head;
  while (s != NULL) {
    sleeper *next = s->siblings.next;

    if (timer_elapsed_ns(&s->timer) >= s->duration_ns) {
      if (s->w->waiter.wake != NULL) {
        s->w->waiter.wake(s->w->waiter.udata);
      }

      if (s->siblings.prev != NULL) {
        s->siblings.prev->siblings.next = s->siblings.next;
      }
      else {
        self->head = s->siblings.next;
      }

      if (s->siblings.next != NULL) {
        s->siblings.next->siblings.prev = s->siblings.prev;
      }
      else {
        self->tail = s->siblings.prev;
      }

      self->count -= 1;

      deallocate(a, s, sizeof(sleeper));
    }

    s = next;
  }

  spinlock_release(&self->lock);
}


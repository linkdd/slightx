#include <klibc/assert.h>
#include <klibc/sync/lock.h>

#include <kernel/proc/scheduler.h>
#include <kernel/chrono/time.h>
#include <kernel/mem/heap.h>
#include <kernel/mem/vmm.h>
#include <kernel/cpu/mp.h>
#include <kernel/halt.h>


static spinlock scheduler_lock = {};
static u32      next_task_id   = 1;


// MARK: - runqueue

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


static task *runqueue_find_by_id(runqueue *self, u32 tid) {
  assert(self != NULL);

  spinlock_acquire(&self->lock);

  for (
    task *iter = self->head;
    iter != NULL;
    iter = iter->scheduling.siblings.next
  ) {
    if (iter->id == tid) {
      spinlock_release(&self->lock);
      return iter;
    }
  }

  spinlock_release(&self->lock);
  return NULL;
}


// MARK: - sleepers

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


// MARK: - scheduler

static void idle_task_fn(void *arg) {
  (void)arg;

  halt();
}


void scheduler_init(void) {
  spinlock_init(&scheduler_lock);
}


void scheduler_load(void) {
  percpu_data *cpu_data = mp_get_percpu_data();

  runqueue_init   (&cpu_data->scheduler.tasks);
  runqueue_init   (&cpu_data->scheduler.cleanup);
  sleeperlist_init(&cpu_data->scheduler.sleepers);

  cpu_data->scheduler.current = NULL;
  cpu_data->scheduler.idle    = NULL;

  allocator a = heap_allocator();

  task *idle_task = allocate(a, sizeof(task));

  spinlock_acquire(&scheduler_lock);
  task_desc idle_task_desc = {
    .task_id     = next_task_id++,
    .parent_task = NULL,
    .kstack_size = 16 * 1024,
    .ustack_size = 0,
    .parent_pmap = NULL,
    .entrypoint  = { .fn = idle_task_fn, .arg = NULL },
    .pin         = { .enabled = true, .processor_id = cpu_data->processor_id },
    .flags       = TH_TASK_FLAG_KERNEL | TH_TASK_FLAG_DETACHED,
  };
  spinlock_release(&scheduler_lock);

  task_init(idle_task, &idle_task_desc);
  task_set_ready(idle_task);

  cpu_data->scheduler.idle    = idle_task;
  cpu_data->scheduler.current = idle_task;
}


u32 scheduler_get_next_tid(void) {
  spinlock_acquire(&scheduler_lock);
  u32 tid = next_task_id++;
  spinlock_release(&scheduler_lock);
  return tid;
}


task *scheduler_get_current_task(void) {
  percpu_data *cpu_data = mp_get_percpu_data();

  return cpu_data->scheduler.current;
}


task *scheduler_get_task_by_id(u32 tid) {
  usize cpu_count = mp_get_cpu_count();

  for (usize cpu_idx = 0; cpu_idx < cpu_count; cpu_idx++) {
    percpu_data *cpu_data = mp_get_percpu_data_of(cpu_idx);
    runqueue    *rq       = &cpu_data->scheduler.tasks;

    task *t = runqueue_find_by_id(rq, tid);
    if (t != NULL) {
      return t;
    }
  }

  return NULL;
}


static void scheduler_wake_task(void *udata) {
  task *t = (task *)udata;
  assert(t != NULL);

  task_set_ready(t);

  percpu_data *cpu_data = NULL;

  if (t->pin.enabled) {
    cpu_data = mp_get_percpu_data_of(t->pin.processor_id);
  }
  else {
    cpu_data = mp_get_percpu_data_of(t->scheduling.last_processor_id);
  }

  runqueue_enqueue(&cpu_data->scheduler.tasks, t);
}


waiter scheduler_make_waiter(task *t) {
  assert(t != NULL);

  return (waiter){
    .wake  = scheduler_wake_task,
    .udata = (void *)t,
  };
}


void scheduler_wakeup_after(u64 ns, waitqueue_item *wait) {
  assert(wait != NULL);

  percpu_data *cpu_data = mp_get_percpu_data();
  sleeperlist_add(&cpu_data->scheduler.sleepers, ns, wait);
}


void scheduler_schedule_for_cleanup(task *t) {
  assert(t != NULL);

  percpu_data *cpu_data = mp_get_percpu_data();
  runqueue_enqueue(&cpu_data->scheduler.cleanup, t);
}


void scheduler_cleanup(void) {
  percpu_data *cpu_data = mp_get_percpu_data();

  task *t = cpu_data->scheduler.cleanup.head;
  while (t != NULL) {
    task *next = t->scheduling.siblings.next;
    task *prev = t->scheduling.siblings.prev;

    if (
      (t->flags & TH_TASK_FLAG_DETACHED) != 0 ||
      waitqueue_is_empty(&t->lifecycle.joiners)
    ) {
      if (prev != NULL) {
        prev->scheduling.siblings.next = next;
      }
      else {
        cpu_data->scheduler.cleanup.head = next;
      }

      if (next != NULL) {
        next->scheduling.siblings.prev = prev;
      }
      else {
        cpu_data->scheduler.cleanup.tail = prev;
      }

      cpu_data->scheduler.cleanup.count -= 1;

      task_deinit(t);
    }

    t = next;
  }
}


void scheduler_yield(void) {
  __asm__ volatile("cli" ::: "memory");

  percpu_data *cpu_data = mp_get_percpu_data();
  task        *old_task = cpu_data->scheduler.current;

  if (old_task != NULL && old_task->state.type == TH_TASK_STATE_RUNNING) {
    task_set_ready(old_task);
    runqueue_enqueue(&cpu_data->scheduler.tasks, old_task);
  }

  task *new_task = runqueue_dequeue(&cpu_data->scheduler.tasks);
  if (new_task == NULL) {
    new_task = cpu_data->scheduler.idle;
  }
  assert(new_task != NULL);

  cpu_data->scheduler.current = new_task;

  if (new_task != cpu_data->scheduler.idle) {
    task_set_running(new_task, cpu_data->processor_id);
  }

  if (old_task != new_task) {
    vmm_switch_page_map(new_task->pmap);

    u64 kstack_top               = (u64)new_task->kstack.base + new_task->kstack.size;
    cpu_data->tss.rsp0           = kstack_top;
    cpu_data->syscall_kernel_rsp = kstack_top;

    task_context_switch(&old_task->context, &new_task->context);
  }
}


[[noreturn]] void scheduler_idle_loop(void) {
  while (true) {
    __asm__ volatile("sti; hlt; cli" ::: "memory");
    scheduler_yield();
  }
}

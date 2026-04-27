#include <klibc/assert.h>

#include <kernel/proc/scheduler/controller.h>
#include <kernel/mem/heap.h>
#include <kernel/cpu/mp.h>
#include <kernel/halt.h>


static spinlock scheduler_lock = {};
static tid      next_task_id   = 1;


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
    .kstack_size = 16 * 1024,
    .ustack_size = 0,
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


void scheduler_tick(u64 ns) {
  percpu_data *cpu = mp_get_percpu_data();

  cpu->scheduler.uptime_ns += ns;

  sleeperlist_tick (&cpu->scheduler.sleepers);
  scheduler_cleanup();
}


tid scheduler_get_next_tid(void) {
  spinlock_acquire(&scheduler_lock);
  tid tid = next_task_id++;
  spinlock_release(&scheduler_lock);
  return tid;
}


task *scheduler_get_current_task(void) {
  percpu_data *cpu_data = mp_get_percpu_data();

  return cpu_data->scheduler.current;
}


task *scheduler_get_task_by_id(tid tid) {
  usize cpu_count = mp_get_cpu_count();

  for (usize cpu_idx = 0; cpu_idx < cpu_count; cpu_idx++) {
    percpu_data *cpu_data = mp_get_percpu_data_of(cpu_idx);

    task *t = runqueue_find_by_id(&cpu_data->scheduler.tasks, tid);
    if (t != NULL) return t;

    t = runqueue_find_by_id(&cpu_data->scheduler.cleanup, tid);
    if (t != NULL) return t;

    task *cur = __atomic_load_n(&cpu_data->scheduler.current, __ATOMIC_ACQUIRE);
    if (cur != NULL && cur->id == tid && task_try_acquire(cur)) {
      if (cur->id == tid) return cur;
      task_release(cur);
    }
  }

  return NULL;
}


void scheduler_kill_current_task(i32 exit_code) {
  percpu_data *cpu_data     = mp_get_percpu_data();
  task        *current_task = cpu_data->scheduler.current;

  task_set_zombie   (current_task, exit_code);
  waitqueue_wake_all(&current_task->lifecycle.joiners);

  __asm__ volatile("cli" ::: "memory");
  runqueue_enqueue(&cpu_data->scheduler.cleanup, current_task);

  task_release(current_task);

  scheduler_yield();
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


void scheduler_cleanup(void) {
  percpu_data *cpu_data = mp_get_percpu_data();

  task *t = cpu_data->scheduler.cleanup.head;
  while (t != NULL) {
    task *next = t->scheduling.siblings.next;
    task *prev = t->scheduling.siblings.prev;

    if (atomic_load_explicit(&t->refcount, memory_order_acquire) == 0) {
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
      deallocate(heap_allocator(), t, sizeof(task));
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

  __atomic_store_n(&cpu_data->scheduler.current, new_task, __ATOMIC_RELEASE);

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

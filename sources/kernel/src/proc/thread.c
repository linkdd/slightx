#include <klibc/assert.h>

#include <kernel/proc/thread.h>
#include <kernel/proc/scheduler.h>


u32 thread_current_id(void) {
  task *current_task = scheduler_get_current_task();
  assert(current_task != NULL);

  return current_task->id;
}


void thread_sleep(u64 ns) {
  task *current_task = scheduler_get_current_task();
  assert(current_task != NULL);

  waitqueue_item_init(
    &current_task->lifecycle.blocker,
    scheduler_make_waiter(current_task)
  );

  __asm__ volatile("cli" ::: "memory");
  scheduler_wakeup_after(ns, &current_task->lifecycle.blocker);
  task_set_blocked      (current_task);
  scheduler_yield       ();
  __asm__ volatile("sti" ::: "memory");
}


void thread_join(u32 tid) {
  task *current_task = scheduler_get_current_task();
  assert(current_task != NULL);

  task *target_task = scheduler_get_task_by_id(tid);
  assert(target_task != NULL);

  assert_release((target_task->flags & TH_TASK_FLAG_DETACHED) == 0);

  if (target_task->state.type == TH_TASK_STATE_ZOMBIE) {
    return;
  }

  waitqueue_item_init(
    &current_task->lifecycle.blocker,
    scheduler_make_waiter(current_task)
  );

  waitqueue_add(
    &target_task->lifecycle.joiners,
    &current_task->lifecycle.blocker
  );

  __asm__ volatile("cli" ::: "memory");
  task_set_blocked(current_task);
  scheduler_yield ();
  __asm__ volatile("sti" ::: "memory");
}


[[noreturn]] void thread_exit_from_task(i32 exit_code) {
  task *current_task = scheduler_get_current_task();
  assert(current_task != NULL);

  task_set_zombie   (current_task, exit_code);
  waitqueue_wake_all(&current_task->lifecycle.joiners);

  __asm__ volatile("cli" ::: "memory");
  scheduler_schedule_for_cleanup(current_task);
  scheduler_yield();

  unreachable();
}

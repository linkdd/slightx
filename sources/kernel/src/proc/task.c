#include <klibc/assert.h>
#include <klibc/mem/bytes.h>

#include <kernel/proc/task.h>
#include <kernel/mem/heap.h>


extern void task_entrypoint_trampoline(void);


void task_init(task *self, const task_desc *desc) {
  assert(self != NULL);
  assert(desc != NULL);
  assert(
    ((desc->flags & TH_TASK_FLAG_KERNEL) != 0 && desc->ustack_size == 0) ||
    ((desc->flags & TH_TASK_FLAG_KERNEL) == 0 && desc->ustack_size > 0)
  );

  allocator a = heap_allocator();

  self->id    = desc->task_id;
  self->pin   = desc->pin;
  self->flags = desc->flags;

  self->state = (task_state){
    .type = TH_TASK_STATE_NEW,
    .data = { .new = {} },
  };

  memset(&self->context, 0, sizeof(task_context));

  usize kstack_size = desc->kstack_size;
  if ((desc->flags & TH_TASK_FLAG_GUARDPAGE) != 0) {
    kstack_size += MM_VIRT_PAGE_SIZE;
  }

  self->kstack.size  = desc->kstack_size;
  self->kstack.base  = allocate_aligned(a, kstack_size, MM_VIRT_PAGE_SIZE);
  self->kstack.limit = ((desc->flags & TH_TASK_FLAG_GUARDPAGE) != 0
    ? (void *)((uptr)self->kstack.base + MM_VIRT_PAGE_SIZE)
    : self->kstack.base
  );

  if ((desc->flags & TH_TASK_FLAG_KERNEL) != 0) {
    self->ustack.size  = 0;
    self->ustack.base  = NULL;
    self->ustack.limit = NULL;
  }
  else {
    self->ustack.size  = desc->ustack_size;
    self->ustack.base  = allocate_aligned(a, self->ustack.size, MM_VIRT_PAGE_SIZE);
    self->ustack.limit = self->ustack.base;
  }

  self->pmap = allocate(a, sizeof(page_map));
  if (desc->parent_pmap != NULL) {
    vmm_clone_page_map(self->pmap, desc->parent_pmap);
  }
  else {
    vmm_make_page_map(self->pmap);
  }

  self->scheduling.last_processor_id = 0;
  self->scheduling.siblings.prev     = NULL;
  self->scheduling.siblings.next     = NULL;

  waitqueue_init(&self->lifecycle.joiners);
  self->lifecycle.parent = desc->parent_task;
  memset(&self->lifecycle.blocker, 0, sizeof(waitqueue_item));

  u64 stack_top  = (u64)self->kstack.base + self->kstack.size;
  stack_top     &= ~0xFULL; // Align to 16 bytes

  u64 *stack = (u64 *)stack_top;
  *(--stack) = (u64)desc->entrypoint.arg;
  *(--stack) = (u64)desc->entrypoint.fn;

  self->context.rsp    = (u64)stack;
  self->context.rbp    = 0;
  self->context.rbx    = 0;
  self->context.r12    = 0;
  self->context.r13    = 0;
  self->context.r14    = 0;
  self->context.r15    = 0;
  self->context.rip    = (u64)task_entrypoint_trampoline;
  self->context.rflags = 0x202; // Enable interrupts
}


void task_deinit(task *self) {
  assert(self != NULL);

  allocator a = heap_allocator();

  vmm_destroy_page_map(self->pmap);
  deallocate(a, self->pmap, sizeof(page_map));

  usize kstack_size = self->kstack.size;
  if ((self->flags & TH_TASK_FLAG_GUARDPAGE) != 0) {
    kstack_size += MM_VIRT_PAGE_SIZE;
  }
  deallocate(a, self->kstack.base, kstack_size);

  if ((self->flags & TH_TASK_FLAG_KERNEL) == 0) {
    deallocate(a, self->ustack.base, self->ustack.size);
  }

  memset(self, 0, sizeof(task));
}

void task_set_ready(task *self) {
  assert(self != NULL);

  self->state = (task_state){
    .type = TH_TASK_STATE_READY,
    .data = { .ready = {} },
  };
}


void task_set_running(task *self, usize processor_id) {
  assert(self != NULL);

  self->state = (task_state){
    .type = TH_TASK_STATE_RUNNING,
    .data = { .running = {
      .processor_id = processor_id,
    } },
  };

  self->scheduling.last_processor_id = processor_id;
}


void task_set_blocked(task *self) {
  assert(self != NULL);

  self->state = (task_state){
    .type = TH_TASK_STATE_BLOCKED,
    .data = { .blocked = {} },
  };
}


void task_set_zombie(task *self, i32 exit_code) {
  assert(self != NULL);

  self->state = (task_state){
    .type = TH_TASK_STATE_ZOMBIE,
    .data = { .zombie = {
      .exit_code = exit_code,
    } },
  };
}

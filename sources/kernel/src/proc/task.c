#include <klibc/assert.h>
#include <klibc/mem/bytes.h>

#include <kernel/proc/task.h>
#include <kernel/mem/heap.h>
#include <kernel/boot/gdt.h>


extern void task_entrypoint_trampoline(void);
extern void task_usermode_trampoline(void);

#define USER_STACK_TOP   0x00007FFFFFFFE000ULL


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

  // For user tasks, map the user stack into the task's address space
  if ((desc->flags & TH_TASK_FLAG_KERNEL) == 0) {
    usize ustack_pages = self->ustack.size / MM_VIRT_PAGE_SIZE;
    uptr  ustack_vbase = USER_STACK_TOP - self->ustack.size;

    page_map *kpmap = vmm_get_kernel_page_map();

    for (usize i = 0; i < ustack_pages; i++) {
      virtual_address  uva     = { .addr = ustack_vbase            + i * MM_VIRT_PAGE_SIZE };
      virtual_address  heap_va = { .addr = (uptr)self->ustack.base + i * MM_VIRT_PAGE_SIZE };
      physical_address upa     = vmm_translate(kpmap, heap_va);

      vmm_map(
        self->pmap, uva, upa,
        MM_PT_FLAG_VALID | MM_PT_FLAG_WRITE | MM_PT_FLAG_USER | MM_PT_FLAG_NX,
        MM_PAGE_4KB
      );
    }
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

  if ((desc->flags & TH_TASK_FLAG_KERNEL) != 0) {
    *(--stack) = (u64)desc->entrypoint.arg;
    *(--stack) = (u64)desc->entrypoint.fn;
    *(--stack) = (u64)task_entrypoint_trampoline;

    self->context.rip = (u64)task_entrypoint_trampoline;
  }
  else {
    u64 user_rsp = USER_STACK_TOP;
    u64 user_cs  = GDT_SEGMENT(GDT_SEG_IDX_UCODE) | RING3;
    u64 user_ss  = GDT_SEGMENT(GDT_SEG_IDX_UDATA) | RING3;

    *(--stack) = user_ss;
    *(--stack) = user_cs;
    *(--stack) = user_rsp;
    *(--stack) = (u64)desc->entrypoint.fn;
    *(--stack) = (u64)task_usermode_trampoline;

    self->context.rip = (u64)task_usermode_trampoline;
  }

  self->context.rsp    = (u64)stack;
  self->context.rbp    = 0;
  self->context.rbx    = 0;
  self->context.r12    = 0;
  self->context.r13    = 0;
  self->context.r14    = 0;
  self->context.r15    = 0;
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

#include <klibc/assert.h>
#include <klibc/mem/align.h>
#include <klibc/mem/bytes.h>

#include <kernel/proc/task.h>
#include <kernel/proc/scheduler/controller.h>

#include <kernel/mem/pmm.h>
#include <kernel/mem/vmm.h>
#include <kernel/mem/heap.h>
#include <kernel/mem/hhdm.h>

#include <kernel/boot/gdt.h>


extern void task_kernelmode_trampoline(void);
extern void task_usermode_trampoline  (void);

#define USER_STACK_TOP   0x00007FFFFFFFE000ULL
#define USER_ARG_BASE    0x00007FFFFF000000ULL


static task_mapping *task_add_mapping(task *self) {
  assert(self != NULL);

  task_mapping *mapping = allocate(heap_allocator(), sizeof(task_mapping));

  mapping->next = self->mappings;
  if (self->mappings != NULL) {
    self->mappings->prev = mapping;
  }
  mapping->prev  = NULL;
  self->mappings = mapping;

  return mapping;
}


static bool task_mapping_overlaps(task *self, uptr base, usize length) {
  assert(self != NULL);

  uptr end = base + length;

  for (task_mapping *m = self->mappings; m != NULL; m = m->next) {
    uptr m_base = m->vaddr.addr;
    uptr m_end  = m_base + m->page_count * MM_VIRT_PAGE_SIZE;

    if (!(end <= m_base || base >= m_end)) {
      return true;
    }
  }

  return false;
}


static void task_del_mapping(task *self, task_mapping *mapping) {
  assert(self != NULL);
  assert(mapping != NULL);

  if (mapping->prev != NULL) mapping->prev->next = mapping->next;
  else                       self->mappings      = mapping->next;
  if (mapping->next != NULL) mapping->next->prev = mapping->prev;

  deallocate(heap_allocator(), mapping, sizeof(task_mapping));
}


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
  self->uid   = desc->uid;
  self->gid   = desc->gid;

  self->state = (task_state){
    .type = TH_TASK_STATE_NEW,
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
  vmm_make_page_map(self->pmap);
  self->mappings = NULL;

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
  memset(&self->lifecycle.blocker, 0, sizeof(waitqueue_item));

  u64 stack_top  = (u64)self->kstack.base + self->kstack.size;
  stack_top     &= ~0xFULL; // Align to 16 bytes

  u64 *stack = (u64 *)stack_top;

  if ((desc->flags & TH_TASK_FLAG_KERNEL) != 0) {
    *(--stack) = (u64)desc->entrypoint.arg;
    *(--stack) = (u64)desc->entrypoint.fn;
    *(--stack) = (u64)task_kernelmode_trampoline;

    self->context.rip = (u64)task_kernelmode_trampoline;
  }
  else {
    u64 user_arg = 0;

    if (desc->entrypoint.arg != NULL) {
      usize arg_len    = strlen((const char *)desc->entrypoint.arg) + 1;
      usize page_count = align_size_up(arg_len, MM_VIRT_PAGE_SIZE) / MM_VIRT_PAGE_SIZE;

      for (usize i = 0; i < page_count; i++) {
        physical_address paddr = pmm_alloc(1);
        virtual_address  hhdm  = hhdm_p2v(paddr);

        usize offset   = i * MM_VIRT_PAGE_SIZE;
        usize copy_len = arg_len - offset;
        if (copy_len > MM_VIRT_PAGE_SIZE) copy_len = MM_VIRT_PAGE_SIZE;

        memcpy(hhdm.ptr, (const u8 *)desc->entrypoint.arg + offset, copy_len);
        if (copy_len < MM_VIRT_PAGE_SIZE) {
          memset((u8 *)hhdm.ptr + copy_len, 0, MM_VIRT_PAGE_SIZE - copy_len);
        }

        virtual_address uva = { .addr = USER_ARG_BASE + i * MM_VIRT_PAGE_SIZE };
        vmm_map(
          self->pmap, uva, paddr,
          MM_PT_FLAG_VALID | MM_PT_FLAG_USER | MM_PT_FLAG_NX,
          MM_PAGE_4KB
        );
      }

      user_arg = USER_ARG_BASE;
    }

    u64 user_rsp = USER_STACK_TOP;
    u64 user_cs  = GDT_SEGMENT(GDT_SEG_IDX_UCODE) | RING3;
    u64 user_ss  = GDT_SEGMENT(GDT_SEG_IDX_UDATA) | RING3;

    *(--stack) = user_ss;
    *(--stack) = user_cs;
    *(--stack) = user_rsp;
    *(--stack) = (u64)desc->entrypoint.fn;
    *(--stack) = user_arg;
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

  task_mapping *m = self->mappings;
  while (m != NULL) {
    task_mapping *next = m->next;
    deallocate(a, m, sizeof(task_mapping));
    m = next;
  }

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


void *task_mmap(task *self, void *addr, usize length, task_mmap_flags flags) {
  assert(self != NULL);

  if (length == 0) return NULL;

  length = align_size_up(length, MM_VIRT_PAGE_SIZE);

  if ((flags & TASK_MMAP_FIXED) != 0) {
    if (
      addr == NULL                                    ||
      !is_ptr_aligned((uptr)addr, MM_VIRT_PAGE_SIZE)  ||
      task_mapping_overlaps(self, (uptr)addr, length)
    ) {
      return NULL;
    }
  }
  else {
    uptr search_base  = 0x000100000000000ULL;
    uptr search_limit = USER_STACK_TOP;

    bool found = false;

    for (uptr candidate = search_base; candidate + length < search_limit; candidate += MM_VIRT_PAGE_SIZE) {
      if (!task_mapping_overlaps(self, candidate, length)) {
        addr  = (void *)candidate;
        found = true;
        break;
      }
    }

    if (!found) {
      return NULL;
    }
  }

  usize page_count = length / MM_VIRT_PAGE_SIZE;

  if ((flags & TASK_MMAP_ACCESS_READ) != 0) {
    usize mapped_pages = 0;

    for (usize i = 0; i < page_count; ++i) {
      physical_address paddr = pmm_try_alloc(1);
      if (paddr.ptr == NULL) {
        // rollback
        for (usize j = 0; j < mapped_pages; ++j) {
          virtual_address  vaddr = { .addr = (uptr)addr + j * MM_VIRT_PAGE_SIZE };
          physical_address paddr = vmm_translate(self->pmap, vaddr);

          vmm_unmap(self->pmap, vaddr);
          pmm_free (paddr, 1);
        }

        return NULL;
      }

      virtual_address vaddr = { .addr = (uptr)addr + i * MM_VIRT_PAGE_SIZE };

      pt_flags page_flags = MM_PT_FLAG_VALID | MM_PT_FLAG_USER;
      if ((flags & TASK_MMAP_ACCESS_WRITE) != 0) page_flags |= MM_PT_FLAG_WRITE;
      if ((flags & TASK_MMAP_ACCESS_EXEC)  == 0) page_flags |= MM_PT_FLAG_NX;

      vmm_map(self->pmap, vaddr, paddr, page_flags, MM_PAGE_4KB);
      mapped_pages++;
    }
  }

  task_mapping *mapping = task_add_mapping(self);
  mapping->vaddr.ptr    = addr;
  mapping->flags        = flags;
  mapping->page_count   = page_count;

  return addr;
}


void task_munmap(task *self, void *addr, usize length) {
  assert(self != NULL);

  if (length == 0 || addr == NULL) return;

  uptr  unmap_base = (uptr)addr;
  usize unmap_len  = align_size_up(length, MM_VIRT_PAGE_SIZE);
  uptr  unmap_end  = unmap_base + unmap_len;

  for (task_mapping *m = self->mappings; m != NULL; m = m->next) {
    uptr m_base = m->vaddr.addr;
    uptr m_end  = m_base + m->page_count * MM_VIRT_PAGE_SIZE;

    if (unmap_base == m_base && unmap_end == m_end) {
      if ((m->flags & TASK_MMAP_ACCESS_READ) != 0) {
        for (usize i = 0; i < m->page_count; ++i) {
          virtual_address  vaddr = { .addr = m_base + i * MM_VIRT_PAGE_SIZE };
          physical_address paddr = vmm_translate(self->pmap, vaddr);

          vmm_unmap(self->pmap, vaddr);
          pmm_free (paddr, 1);
        }
      }

      task_del_mapping(self, m);
      return;
    }
  }
}


tid task_current_id(void) {
  task *current_task = scheduler_get_current_task();
  assert(current_task != NULL);

  return current_task->id;
}


void task_sleep(u64 ns) {
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


void task_join(tid tid) {
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


[[noreturn]] void task_exit(i32 exit_code) {
  scheduler_kill_current_task(exit_code);
  unreachable();
}

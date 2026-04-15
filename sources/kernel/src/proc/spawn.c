#include <klibc/assert.h>
#include <klibc/mem/bytes.h>

#include <kernel/proc/spawn.h>
#include <kernel/proc/scheduler/controller.h>

#include <kernel/mem/heap.h>
#include <kernel/mem/pmm.h>
#include <kernel/mem/vmm.h>
#include <kernel/mem/hhdm.h>

#include <kernel/cpu/mp.h>


tid spawn_kernel_task(task_entrypoint entrpoint) {
  allocator    a        = heap_allocator();
  percpu_data *cpu_data = mp_get_percpu_data();

  task *t = allocate(a, sizeof(task));

  task_desc desc = {
    .task_id     = scheduler_get_next_tid(),
    .kstack_size = 16 * 1024,
    .ustack_size = 0,
    .entrypoint  = entrpoint,
    .pin         = { .enabled = false },
    .flags       = TH_TASK_FLAG_KERNEL,
    .uid         = 0,
    .gid         = 0,
  };

  task_init       (t, &desc);
  task_set_ready  (t);
  runqueue_enqueue(&cpu_data->scheduler.tasks, t);

  return desc.task_id;
}


tid spawn_user_task(const_span binary, const char *arg) {
  assert(binary.data != NULL);
  assert(binary.size > 0);

  allocator    a        = heap_allocator();
  percpu_data *cpu_data = mp_get_percpu_data();

  task *current_task = scheduler_get_current_task();

  usize             bin_page_count  = (binary.size + MM_VIRT_PAGE_SIZE - 1) / MM_VIRT_PAGE_SIZE;
  physical_address *bin_page_paddrs = allocate(a, sizeof(physical_address) * bin_page_count);

  for (usize i = 0; i < bin_page_count; i++) {
    bin_page_paddrs[i] = pmm_alloc(1);
    virtual_address page_va = hhdm_p2v(bin_page_paddrs[i]);

    usize offset   = i * MM_VIRT_PAGE_SIZE;
    usize copy_len = binary.size - offset;
    if (copy_len > MM_VIRT_PAGE_SIZE) copy_len = MM_VIRT_PAGE_SIZE;

    memcpy(page_va.ptr, (const u8 *)binary.data + offset, copy_len);
    if (copy_len < MM_VIRT_PAGE_SIZE) {
      memset((u8 *)page_va.ptr + copy_len, 0, MM_VIRT_PAGE_SIZE - copy_len);
    }
  }

  task *t = allocate(a, sizeof(task));

  task_desc desc = {
    .task_id     = scheduler_get_next_tid(),
    .kstack_size = 16 * 1024,
    .ustack_size = 16 * 1024,
    .entrypoint  = { .fn = (void (*)(void *))USER_CODE_BASE, .arg = (void*) arg },
    .pin         = { .enabled = false },
    .flags       = TH_TASK_FLAG_DETACHED,
    .uid         = (current_task != NULL ? current_task->uid : 0),
    .gid         = (current_task != NULL ? current_task->gid : 0),
  };

  task_init(t, &desc);

  for (usize i = 0; i < bin_page_count; i++) {
    virtual_address bin_page_vaddr = { .addr = USER_CODE_BASE + i * MM_VIRT_PAGE_SIZE };
    vmm_map(
      t->pmap, bin_page_vaddr, bin_page_paddrs[i],
      MM_PT_FLAG_VALID | MM_PT_FLAG_USER,
      MM_PAGE_4KB
    );
  }

  deallocate(a, bin_page_paddrs, sizeof(physical_address) * bin_page_count);

  task_set_ready(t);
  runqueue_enqueue(&cpu_data->scheduler.tasks, t);

  return desc.task_id;
}

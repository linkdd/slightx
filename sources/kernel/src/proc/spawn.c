#include <klibc/assert.h>
#include <klibc/mem/bytes.h>

#include <kernel/proc/spawn.h>
#include <kernel/proc/scheduler.h>

#include <kernel/mem/heap.h>
#include <kernel/mem/pmm.h>
#include <kernel/mem/vmm.h>
#include <kernel/mem/hhdm.h>

#include <kernel/cpu/mp.h>

#include <kernel/vfs/main.h>


#define USER_CODE_BASE  0x400000ULL


tid spawn_kernel_task(task_entrypoint entrpoint) {
  allocator    a        = heap_allocator();
  percpu_data *cpu_data = mp_get_percpu_data();

  task *t = allocate(a, sizeof(task));

  task_desc desc = {
    .task_id     = scheduler_get_next_tid(),
    .parent_task = NULL,
    .kstack_size = 16 * 1024,
    .ustack_size = 0,
    .parent_pmap = NULL,
    .entrypoint  = entrpoint,
    .pin         = { .enabled = false },
    .flags       = TH_TASK_FLAG_KERNEL,
  };

  task_init       (t, &desc);
  task_set_ready  (t);
  runqueue_enqueue(&cpu_data->scheduler.tasks, t);

  return desc.task_id;
}


tid spawn_user_task(const_span binary) {
  assert(binary.data != NULL);
  assert(binary.size > 0);

  allocator    a        = heap_allocator();
  percpu_data *cpu_data = mp_get_percpu_data();

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
    .parent_task = NULL,
    .kstack_size = 16 * 1024,
    .ustack_size = 16 * 1024,
    .parent_pmap = NULL,
    .entrypoint  = { .fn = (void (*)(void *))USER_CODE_BASE, .arg = NULL },
    .pin         = { .enabled = false },
    .flags       = TH_TASK_FLAG_DETACHED,
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


RESULT(tid, str) spawn_executable(str path) {
  auto stat = vfs_stat(path);
  if (!stat.is_ok) {
    return (RESULT(tid, str)) ERR(vfs_strerror(stat.err));
  }

  auto f = vfs_open(path, VFS_O_READ);
  if (!f.is_ok) {
    return (RESULT(tid, str)) ERR(vfs_strerror(f.err));
  }

  usize  bufsz  = (usize)stat.ok.size;
  void  *buffer = allocate(heap_allocator(), bufsz);
  span   binary = make_span(buffer, bufsz);

  auto read = vfs_read(f.ok, binary);
  assert(vfs_close(f.ok).is_ok);

  if (!read.is_ok) {
    return (RESULT(tid, str)) ERR(vfs_strerror(read.err));
  }

  u32 tid = spawn_user_task(span_as_const(binary));
  deallocate(heap_allocator(), buffer, bufsz);

  return (RESULT(tid, str)) OK(tid);
}

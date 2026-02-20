#include <limine.h>

#include <klibc/assert.h>
#include <klibc/types.h>
#include <klibc/io/log.h>

#include <kernel/halt.h>

#include <kernel/drivers/console.h>

#include <kernel/boot/gdt.h>
#include <kernel/boot/idt.h>
#include <kernel/boot/exc.h>
#include <kernel/boot/isr.h>
#include <kernel/boot/irq.h>
#include <kernel/boot/pic.h>
#include <kernel/boot/tss.h>
#include <kernel/boot/acpi.h>
#include <kernel/boot/madt.h>
#include <kernel/boot/lapic.h>

#include <kernel/mem/pmm.h>
#include <kernel/mem/vmm.h>
#include <kernel/mem/heap.h>

#include <kernel/cpu/mp.h>

#include <kernel/proc/scheduler.h>
#include <kernel/proc/thread.h>


static void init_static_globals(void) {
  klogger_init();

  gdt_init();
  isr_init();
  irq_init();
  exc_init();

  pmm_init ();
  vmm_init ();
  heap_init();

  mp_init();

  scheduler_init();
}


static void ap_start(void) {
  gdt_load();
  tss_load();
  idt_load();
  pic_load();

  lapic_configure_timer();

  scheduler_load();

  asm("sti");

  syncpoint *sync = mp_get_syncpoint();
  syncpoint_notify(sync);

  scheduler_idle_loop();
}


static void bootstrap(void) {
  gdt_load();
  idt_load();
  pic_load();

  pmm_load ();
  vmm_load ();
  heap_load();

  mp_load ();
  tss_load();

  acpi_load     ();
  madt_mmio_load();

  lapic_calibrate      ();
  lapic_configure_timer();

  scheduler_load();

  asm("sti");

  syncpoint *sync = mp_get_syncpoint();
  syncpoint_set(sync, mp_get_cpu_count() - 1);

  mp_ap_jump(ap_start);

  syncpoint_wait(sync);
}


static void dummy_task(void *arg) {
  u8 c = (u8)(u64)arg;

  while (true) {
    klog("%c", (i32)c);
    thread_sleep(1 * 1'000'000);
  }
}


void kmain(void) {
  console_init();

  init_static_globals();
  bootstrap();

  LIMINE_GET_RESP(bootloader_info);
  assert_release(bootloader_info_response != NULL);
  klog(
    "bootloader: %s %s",
    strview_from_cstr(bootloader_info_response->name),
    strview_from_cstr(bootloader_info_response->version)
  );

  LIMINE_GET_RESP(executable_file);
  assert_release(executable_file_response != NULL);
  klog(
    "[slightx]# %s %s",
    strview_from_cstr(executable_file_response->executable_file->path),
    strview_from_cstr(executable_file_response->executable_file->string)
  );

  allocator a = heap_allocator();

  task      *t1 = allocate(a, sizeof(task));
  task_desc  d1 = {
    .task_id     = scheduler_get_next_tid(),
    .parent_task = NULL,
    .kstack_size = 16 * 1024,
    .ustack_size = 0,
    .parent_pmap = NULL,
    .entrypoint  = { .fn = dummy_task, .arg = (void*)(u64)('-') },
    .pin         = { .enabled = false },
    .flags       = TH_TASK_FLAG_KERNEL,
  };
  task_init(t1, &d1);
  task_set_ready(t1);

  percpu_data *cpu_data = mp_get_percpu_data();
  runqueue_enqueue(&cpu_data->scheduler.tasks, t1);

  task      *t2 = allocate(a, sizeof(task));
  task_desc  d2 = {
    .task_id     = scheduler_get_next_tid(),
    .parent_task = NULL,
    .kstack_size = 16 * 1024,
    .ustack_size = 0,
    .parent_pmap = NULL,
    .entrypoint  = { .fn = dummy_task, .arg = (void*)(u64)('+') },
    .pin         = { .enabled = false },
    .flags       = TH_TASK_FLAG_KERNEL,
  };
  task_init(t2, &d2);
  task_set_ready(t2);

  runqueue_enqueue(&cpu_data->scheduler.tasks, t2);

  scheduler_yield    ();
  scheduler_idle_loop();
}

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

#include <kernel/mem/pmm.h>
#include <kernel/mem/vmm.h>
#include <kernel/mem/heap.h>

#include <kernel/cpu/mp.h>


static void init_static_globals(void) {
  gdt_init();
  isr_init();
  irq_init();
  exc_init();

  pmm_init ();
  vmm_init ();
  heap_init();

  mp_init();
}


static void ap_start(void) {
  gdt_load();
  tss_load();
  idt_load();
  pic_load();

  asm("sti");

  syncpoint *sync = mp_get_syncpoint();
  syncpoint_notify(sync);

  halt();
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

  asm("sti");

  syncpoint *sync = mp_get_syncpoint();
  syncpoint_set(sync, mp_get_cpu_count() - 1);

  mp_ap_jump(ap_start);

  syncpoint_wait(sync);
}


void kmain(void) {
  console_init();

  init_static_globals();
  bootstrap();

  LIMINE_GET_RESP(bootloader_info);
  assert_release(bootloader_info_response != NULL);
  klog(
    "bootloader: %s %s\n",
    strview_from_cstr(bootloader_info_response->name),
    strview_from_cstr(bootloader_info_response->version)
  );

  LIMINE_GET_RESP(executable_file);
  assert_release(executable_file_response != NULL);
  klog(
    "[slightx]# %s %s\n",
    strview_from_cstr(executable_file_response->executable_file->path),
    strview_from_cstr(executable_file_response->executable_file->string)
  );

  allocator a = heap_allocator();

  u64 *i = allocate(a, sizeof(u64));
  *i = 42;
  klog("HEAP: %d\n", *i);
  deallocate(a, i, sizeof(u64));

  halt();
}

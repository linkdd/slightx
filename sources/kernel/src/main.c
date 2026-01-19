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

#include <kernel/mem/pmm.h>


static void init_static_globals(void) {
  gdt_init();
  isr_init();
  irq_init();
  exc_init();

  pmm_init();
}


static void bootstrap(void) {
  gdt_load();
  idt_load();
  pic_load();

  pmm_load();

  asm("sti");
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

  halt();
}

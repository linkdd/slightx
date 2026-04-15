#include <kernel/panic.h>
#include <kernel/halt.h>

#include <kernel/boot/lapic.h>
#include <klibc/io/log.h>


[[noreturn]] void panic(const char *fmt, ...) {
  va_list ap;

  __asm__ volatile("cli");
  lapic_send_panic_ipi();

  klog("!!! KERNEL PANIC !!!\r\n");

  va_start(ap, fmt);
  vklog(fmt, ap);
  va_end(ap);

  halt();
}

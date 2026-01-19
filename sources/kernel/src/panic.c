#include <kernel/panic.h>
#include <kernel/halt.h>

#include <klibc/io/log.h>


[[noreturn]] void panic(const char *fmt, ...) {
  va_list ap;

  klog("!!! KERNEL PANIC !!!\n");

  va_start(ap, fmt);
  vklog(fmt, ap);
  va_end(ap);

  halt();
}

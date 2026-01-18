#include <kernel/panic.h>
#include <kernel/halt.h>

#include <klibc/io/log.h>


[[noreturn]] void panic(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vklog(fmt, ap);
  va_end(ap);

  halt();
}

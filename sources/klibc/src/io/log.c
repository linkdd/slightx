#include <klibc/io/log.h>
#include <klibc/mem/str.h>
#include <klibc/algo/conv.h>
#include <klibc/algo/format.h>
#include <klibc/sync/lock.h>


extern void console_write(str s);


static spinlock log_lock = {};


void klogger_init(void) {
  spinlock_init(&log_lock);
}


void klog(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vklog(fmt, ap);
  va_end(ap);
}


static void log_formatter(void *udata, str chunk) {
  (void)udata;
  console_write(chunk);
}


void vklog(const char *fmt, va_list ap) {
  spinlock_acquire(&log_lock);

  formatter f = {
    .consume = log_formatter,
    .udata   = NULL,
  };

  formatter_apply_v(&f, fmt, ap);

  console_write(str_literal("\n"));

  spinlock_release(&log_lock);
}

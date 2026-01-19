#include <klibc/io/log.h>
#include <klibc/mem/str.h>
#include <klibc/algo/conv.h>
#include <klibc/algo/format.h>


extern void console_write(str s);


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
  formatter f = {
    .consume = log_formatter,
    .udata   = NULL,
  };

  formatter_apply_v(&f, fmt, ap);
}

#include <slightx/io.h>
#include <slightx/algo/format.h>
#include <slightx/sys/cap.h>


void puts(str s) {
  sys_send(CONSOLE_CAP_ID, make_const_span(s.data, s.length));
}


void print(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  printv(fmt, args);
  va_end(args);
}


static void str_formatter(void *udata, str chunk) {
  (void)udata;
  puts(chunk);
}


void printv(const char *fmt, va_list ap) {
  formatter f = {
    .consume = str_formatter,
    .udata   = NULL,
  };

  formatter_apply_v(&f, fmt, ap);
}

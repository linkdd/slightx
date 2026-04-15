#include <slightx/sys/proc.h>
#include <slightx/sys/io.h>


[[noreturn]] void _start(const char *arg) {
  sys_puts(str_literal("Hello from userland: "));
  if (arg != NULL) {
    sys_puts(strview_from_cstr(arg));
  }
  sys_puts(str_literal("\r\n"));
  sys_exit(0);
}

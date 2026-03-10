#include <slightx/sys/proc.h>
#include <slightx/sys/io.h>


[[noreturn]] void _start(void) {
  sys_puts(str_literal("Hello from userland\n"));
  sys_exit(0);
}

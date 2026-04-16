#include <slightx/sys/io.h>
#include <slightx/os.h>


int main(void) {
  sys_puts(str_literal("Hello from userland: "));

  strv args = os_get_args();
  for (usize i = 0; i < args.count; i++) {
    str arg = args.items[i];
    sys_puts(arg);
    if (i + 1 < args.count) {
      sys_puts(str_literal(" "));
    }
  }

  sys_puts(str_literal("\r\n"));

  return 0;
}

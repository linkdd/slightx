#include <slightx/sys/proc.h>
#include <slightx/sys/io.h>


[[gnu::no_sanitize("function")]]
[[noreturn]] void _start(const task_startup_info *info) {
  sys_puts(str_literal("Hello from userland: "));

  for (usize i = 0; i < info->args.count; i++) {
    str arg = info->args.items[i];
    sys_puts(arg);
    if (i + 1 < info->args.count) {
      sys_puts(str_literal(" "));
    }
  }

  sys_puts(str_literal("\r\n"));
  sys_exit(0);
}

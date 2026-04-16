#include <slightx/assert.h>

#include <slightx/sys/proc.h>
#include <slightx/os.h>


extern int main(void);


static volatile task_startup_info *g_info = NULL;


[[noreturn]] [[gnu::section(".text.boot")]] void _start(task_startup_info *info) {
  assert(info != NULL);
  g_info = info;
  assert(g_info != NULL);
  sys_exit(main());
}


strv os_get_args(void) {
  assert(g_info != NULL);
  return g_info->args;
}


strv os_get_envvars(void) {
  assert(g_info != NULL);
  return g_info->envvars;
}

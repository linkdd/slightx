#include <slightx/io.h>
#include <slightx/os.h>


int main(void) {
  print("Hello from userland:");

  strv args = os_get_args();
  for (usize i = 0; i < args.count; i++) {
    str arg = args.items[i];
    print(" %s", arg);
  }

  print("\r\n");

  return 0;
}

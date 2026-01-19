#include <kernel/halt.h>


[[noreturn]] void halt(void) {
  while (true) {
    asm volatile("hlt");
  }
}

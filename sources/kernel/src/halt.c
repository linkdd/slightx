#include <kernel/halt.h>


[[noreturn]] void halt(void) {
  while (true) {
    __asm__ volatile("hlt");
  }
}

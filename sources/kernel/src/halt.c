#include <kernel/halt.h>


void halt(void) {
  while (true) {
    asm volatile("hlt");
  }
}

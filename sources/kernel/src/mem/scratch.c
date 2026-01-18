#include <kernel/mem/scratch.h>


static constexpr usize capacity = 1024 * 1024; // 1 MiB

static u8    buffer[capacity];
static arena a;


void scratch_init(void) {
  arena_init(&a, make_span(buffer, capacity));
}


arena *scratch_arena(void) {
  return &a;
}

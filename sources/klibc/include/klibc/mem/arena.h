#pragma once

#include <klibc/types.h>
#include <klibc/mem/alloc.h>
#include <klibc/mem/span.h>
#include <klibc/sync/lock.h>


typedef struct arena arena;
struct arena {
  span  memory;
  usize offset;
};


void arena_init (arena *a, span memory);
void arena_reset(arena *a);

allocator arena_allocator(arena *a);

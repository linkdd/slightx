#pragma once

#include <slightx/types.h>
#include <slightx/mem/alloc.h>
#include <slightx/mem/span.h>


typedef struct arena arena;
struct arena {
  span  memory;
  usize offset;
};


void arena_init (arena *a, span memory);
void arena_reset(arena *a);

allocator arena_allocator(arena *a);

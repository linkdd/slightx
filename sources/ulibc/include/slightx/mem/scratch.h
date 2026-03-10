#pragma once

#include <slightx/types.h>
#include <slightx/mem/alloc.h>


typedef struct scratch_alloc_hdr scratch_alloc_hdr;
struct scratch_alloc_hdr {
  scratch_alloc_hdr *prev;
  scratch_alloc_hdr *next;

  usize size;
  u8    data[];
};

typedef struct scratch scratch;
struct scratch {
  allocator a;

  scratch_alloc_hdr *head;
};


void scratch_init  (scratch *self, allocator a);
void scratch_deinit(scratch *self);

allocator scratch_as_allocator(scratch *self);

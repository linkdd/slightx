#include <slightx/mem/arena.h>
#include <slightx/mem/align.h>
#include <slightx/mem/bytes.h>
#include <slightx/assert.h>


void arena_init(arena *a, span memory) {
  assert(a != NULL);
  assert(memory.data != NULL && memory.size > 0);

  a->memory = memory;
  a->offset = 0;
}


void arena_reset(arena *a) {
  assert(a != NULL);

  a->offset = 0;
}


static void *arena_allocate(void *udata, usize sz) {
  arena *a = (arena *)udata;
  assert(a != NULL);

  sz = align_size_up(sz, alignof(max_align_t));
  assert_release(a->memory.size - a->offset >= sz);

  void *ptr  = (void *)((uptr)a->memory.data + a->offset);
  a->offset += sz;

  return ptr;
}


static void *arena_reallocate(void *udata, void *ptr, usize oldsz, usize newsz) {
  if (newsz == 0) {
    return NULL;
  }

  void *newptr = arena_allocate(udata, newsz);

  if (ptr != NULL) {
    usize copy_sz = oldsz < newsz ? oldsz : newsz;
    memcpy(newptr, ptr, copy_sz);
  }

  return newptr;
}


static void arena_deallocate(void *udata, void *ptr, usize sz) {
  (void)udata;
  (void)ptr;
  (void)sz;
  // No-op
}


allocator arena_allocator(arena *a) {
  return (allocator){
    .allocate   = arena_allocate,
    .reallocate = arena_reallocate,
    .deallocate = arena_deallocate,
    .udata      = a,
  };
}

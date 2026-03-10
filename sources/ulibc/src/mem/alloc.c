#include <slightx/mem/alloc.h>
#include <slightx/mem/bytes.h>
#include <slightx/assert.h>


void *allocate(allocator a, usize sz) {
  assert(a.allocate != NULL);

  void *ptr = a.allocate(a.udata, sz);
  assert_release(ptr != NULL);
  memset(ptr, 0, sz);

  return ptr;
}


void *allocate_v(allocator a, usize nmemb, usize membsz) {
  assert(a.allocate != NULL);

  assert_release(SIZE_MAX / nmemb >= membsz);
  usize sz = nmemb * membsz;
  return allocate(a, sz);
}


void *reallocate(allocator a, void *ptr, usize oldsz, usize newsz) {
  assert(a.reallocate != NULL);

  void *newptr = a.reallocate(a.udata, ptr, oldsz, newsz);
  if (newptr != NULL && newsz > oldsz) {
    memset((u8 *)newptr + oldsz, 0, newsz - oldsz);
  }

  return newptr;
}


void deallocate(allocator a, void *ptr, usize sz) {
  assert(a.deallocate != NULL);

  if (ptr != NULL) {
    a.deallocate(a.udata, ptr, sz);
  }
}

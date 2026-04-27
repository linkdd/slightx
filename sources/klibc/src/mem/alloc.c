#include <klibc/mem/alloc.h>
#include <klibc/mem/bytes.h>
#include <klibc/assert.h>


void *allocate(allocator a, usize sz) {
  assert(a.allocate != NULL);
  assert(sz > 0);

  void *ptr = a.allocate(a.udata, sz);
  assert_release(ptr != NULL);
  memset(ptr, 0, sz);

  return ptr;
}


void *allocate_aligned(allocator a, usize sz, usize align) {
  assert(a.allocate_aligned != NULL);
  assert(sz > 0);

  assert_release(align % 2 == 0);
  assert_release(align >= alignof(max_align_t));

  void *ptr = a.allocate_aligned(a.udata, sz, align);
  assert_release(ptr != NULL);
  memset(ptr, 0, sz);

  return ptr;
}


void *allocate_v(allocator a, usize nmemb, usize membsz) {
  assert(a.allocate != NULL);
  assert(nmemb > 0);
  assert(membsz > 0);

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

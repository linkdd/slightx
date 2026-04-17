#include <slightx/mem/alloc.h>
#include <slightx/mem/bytes.h>
#include <slightx/sys/mem.h>
#include <slightx/assert.h>


static void *_default_allocate(void *udata, usize sz) {
  (void)udata;
  return sys_mmap(NULL, sz, MMAP_ACCESS_READ | MMAP_ACCESS_WRITE);
}


static void *_default_reallocate(void *udata, void *ptr, usize oldsz, usize newsz) {
  (void)udata;

  void *newptr = NULL;

  if (newsz > 0) {
    newptr = sys_mmap(NULL, newsz, MMAP_ACCESS_READ | MMAP_ACCESS_WRITE);
  }

  if (newsz > oldsz) {
    memcpy(newptr, ptr, oldsz);
  }

  if (ptr != NULL) {
    sys_munmap(ptr, oldsz);
  }

  return newptr;
}


static void _default_deallocate(void *udata, void *ptr, usize sz) {
  (void)udata;

  if (ptr != NULL) {
    sys_munmap(ptr, sz);
  }
}


allocator default_allocator(void) {
  allocator a = {
    .allocate   = _default_allocate,
    .reallocate = _default_reallocate,
    .deallocate = _default_deallocate,
    .udata      = NULL,
  };

  return a;
}


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

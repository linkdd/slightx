#include <liballoc.h>

#include <slightx/mem/alloc.h>
#include <slightx/mem/bytes.h>
#include <slightx/sys/mem.h>
#include <slightx/assert.h>


static constexpr usize PAGE_SIZE = 4096;


int liballoc_lock(void) {
  return 0;
}


int liballoc_unlock(void) {
  return 0;
}


void *liballoc_alloc(int page_count) {
  return sys_mmap(NULL, page_count * PAGE_SIZE, MMAP_ACCESS_READ | MMAP_ACCESS_WRITE);
}


int liballoc_free(void *ptr, int page_count) {
  sys_munmap(ptr, page_count * PAGE_SIZE);
  return 0;
}


static void *_default_allocate(void *udata, usize sz) {
  (void)udata;
  return malloc(sz);
}


static void *_default_reallocate(void *udata, void *ptr, usize oldsz, usize newsz) {
  (void)udata;
  (void)oldsz;
  return realloc(ptr, newsz);
}


static void _default_deallocate(void *udata, void *ptr, usize sz) {
  (void)udata;
  (void)sz;
  free(ptr);
}


allocator default_allocator(void) {
  return (allocator){
    .allocate   = _default_allocate,
    .reallocate = _default_reallocate,
    .deallocate = _default_deallocate,
    .udata = NULL
  };
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

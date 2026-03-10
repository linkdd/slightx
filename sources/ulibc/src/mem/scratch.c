#include <slightx/mem/scratch.h>
#include <slightx/mem/bytes.h>
#include <slightx/assert.h>


void scratch_init(scratch *self, allocator a) {
  assert(self != NULL);

  self->a    = a;
  self->head = NULL;
}


void scratch_deinit(scratch *self) {
  assert(self != NULL);

  scratch_alloc_hdr *current = self->head;

  while (current != NULL) {
    scratch_alloc_hdr *next = current->next;

    deallocate(self->a, current, sizeof(scratch_alloc_hdr) + current->size);

    current = next;
  }

  memset(self, 0, sizeof(scratch));
}


static inline scratch_alloc_hdr *hdr_from_ptr(void *ptr) {
  assert(ptr != NULL);
  return (scratch_alloc_hdr *)((uptr)ptr - offsetof(scratch_alloc_hdr, data));
}


static void *scratch_allocate(void *udata, usize size) {
  scratch *self = (scratch *)udata;
  assert(self != NULL);

  scratch_alloc_hdr *hdr = allocate(self->a, sizeof(scratch_alloc_hdr) + size);

  hdr->size = size;

  if (self->head == NULL) {
    self->head = hdr;
  }
  else {
    hdr->next       = self->head;
    hdr->next->prev = hdr;
    self->head      = hdr;
  }

  return hdr->data;
}


static void *scratch_reallocate(void *udata, void *ptr, usize old_size, usize new_size) {
  scratch *self = (scratch *)udata;
  assert(self != NULL);

  scratch_alloc_hdr *hdr = hdr_from_ptr(ptr);
  assert(hdr->size == old_size);

  scratch_alloc_hdr *prev = hdr->prev;
  scratch_alloc_hdr *next = hdr->next;

  usize hdr_osize = sizeof(scratch_alloc_hdr) + old_size;
  usize hdr_nsize = sizeof(scratch_alloc_hdr) + new_size;
  hdr             = reallocate(self->a, hdr, hdr_osize, hdr_nsize);
  hdr->size       = new_size;

  if (prev != NULL) prev->next = hdr;
  else              self->head = hdr;
  if (next != NULL) next->prev = hdr;

  return hdr->data;
}


static void scratch_deallocate(void *udata, void *ptr, usize size) {
  scratch *self = (scratch *)udata;
  assert(self != NULL);

  scratch_alloc_hdr *hdr = hdr_from_ptr(ptr);
  assert(hdr->size == size);

  scratch_alloc_hdr *prev = hdr->prev;
  scratch_alloc_hdr *next = hdr->next;

  deallocate(self->a, hdr, sizeof(scratch_alloc_hdr) + size);

  if (prev != NULL) prev->next = next;
  else              self->head = next;
  if (next != NULL) next->prev = prev;
}


allocator scratch_as_allocator(scratch *self) {
  assert(self != NULL);

  return (allocator){
    .allocate   = scratch_allocate,
    .reallocate = scratch_reallocate,
    .deallocate = scratch_deallocate,
    .udata      = self,
  };
}

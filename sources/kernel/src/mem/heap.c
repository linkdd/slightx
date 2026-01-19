#include <liballoc.h>

#include <klibc/collections/bitmap.h>
#include <klibc/mem/align.h>
#include <klibc/mem/bytes.h>
#include <klibc/sync/lock.h>
#include <klibc/io/log.h>
#include <klibc/assert.h>

#include <kernel/mem/heap.h>
#include <kernel/mem/pmm.h>


typedef struct heap_block_hdr heap_block_hdr;
struct heap_block_hdr {
  void  *base;
  usize  size;
  usize  align;
  usize  offset;
};


extern uptr kernel_region_end;

static constexpr usize heap_map_bufalign = 8;
static constexpr usize heap_map_bufsize  = (((HEAP_PAGE_COUNT) + ((heap_map_bufalign) - 1)) / (heap_map_bufalign) * (heap_map_bufalign));
static u8 heap_map_buf[heap_map_bufsize];

static spinlock heap_lock = {};
static bitmap   heap_map  = {};


static inline virtual_address heap_start(void) {
  return (virtual_address){ .addr = align_ptr_up((uptr)&kernel_region_end, MM_VIRT_PAGE_SIZE) };
}


static inline virtual_address heap_end(void) {
  virtual_address start = heap_start();
  return (virtual_address){ .addr = start.addr + HEAP_SIZE };
}


void heap_init(void) {
  spinlock_init(&heap_lock);
}


void heap_load(void) {
  bitmap_init(&heap_map, make_span(heap_map_buf, heap_map_bufsize));

  // guard pages
  bitmap_set(&heap_map, 0);
  bitmap_set(&heap_map, HEAP_PAGE_COUNT - 1);
}


// MARK: liballoc
void *liballoc_alloc(int page_count) {
  usize total_page_count     = HEAP_PAGE_COUNT;
  usize requested_page_count = (usize)page_count;

  for (u64 page_range_start = 0; page_range_start < total_page_count; page_range_start++) {
    bool found = true;

    for (u64 page_offset = 0; page_offset < requested_page_count; page_offset++) {
      u64 page_bit_index = page_range_start + page_offset;

      if (
        page_bit_index >= total_page_count ||
        bitmap_isset(&heap_map, page_bit_index)
      ) {
        page_range_start += page_offset;
        found             = false;
        break;
      }
    }

    if (found) {
      virtual_address heap_base   = heap_start();
      virtual_address block_start = { .addr = heap_base.addr + page_range_start * MM_VIRT_PAGE_SIZE };

      bool success     = true;
      u64  page_offset = 0;

      while (page_offset < requested_page_count) {
        virtual_address  va = { .addr = block_start.addr + page_offset * MM_VIRT_PAGE_SIZE };
        physical_address pa = pmm_alloc(1);

        if (pa.ptr == NULL) {
          klog("[heap] pmm_alloc(): not enough memory");
          success = false;
          break;
        }

        vmm_map(
          vmm_get_kernel_page_map(),
          va,
          pa,
          MM_PT_FLAG_WRITE | MM_PT_FLAG_GLOBAL,
          MM_PAGE_4KB
        );

        bitmap_set(&heap_map, page_range_start + page_offset);
        memset(va.ptr, 0, MM_VIRT_PAGE_SIZE);
        page_offset++;
      }

      if (!success) {
        while (page_offset > 0) {
          page_offset--;

          virtual_address  va = { .addr = block_start.addr + page_offset * MM_VIRT_PAGE_SIZE };
          physical_address pa = vmm_translate(vmm_get_kernel_page_map(), va);

          vmm_unmap(vmm_get_kernel_page_map(), va);
          pmm_free(pa, 1);
          bitmap_clear(&heap_map, page_range_start + page_offset);
        }

        break;
      }

      return block_start.ptr;
    }
  }

  return NULL;
}


int liballoc_free(void *ptr, int page_count) {
  if (ptr == NULL || page_count <= 0) return 0;

  virtual_address heap_base_vaddr = heap_start();
  virtual_address heap_end_vaddr  = heap_end();
  virtual_address block           = { .ptr = ptr };

  if (
    block.addr < heap_base_vaddr.addr ||
    block.addr >= heap_end_vaddr.addr ||
    !is_ptr_aligned(block.addr, MM_VIRT_PAGE_SIZE)
  ) {
    return -1;
  }

  u64   page_range_base        = (block.addr - heap_base_vaddr.addr) / MM_VIRT_PAGE_SIZE;
  usize reclaimable_page_count = (usize)page_count;
  int   rc                     = 0;

  for (u64 page_offset = 0; page_offset < reclaimable_page_count; page_offset++) {
    virtual_address  va = { .addr = block.addr + page_offset * MM_VIRT_PAGE_SIZE };
    physical_address pa = vmm_translate(vmm_get_kernel_page_map(), va);

    vmm_unmap(vmm_get_kernel_page_map(), va);
    pmm_free(pa, 1);
    bitmap_clear(&heap_map, page_range_base + page_offset);
  }

  return rc;
}


int liballoc_lock(void) {
  spinlock_acquire(&heap_lock);
  return 0;
}


int liballoc_unlock(void) {
  spinlock_release(&heap_lock);
  return 0;
}


// MARK: allocator
static void *heap_allocate_aligned(void *udata, usize sz, usize align) {
  (void)udata;

  if (sz == 0)                      return NULL;
  if (!is_alignment_valid(align))   return NULL;
  if (align < alignof(max_align_t)) align = alignof(max_align_t);

  usize  totalsz = sz + sizeof(heap_block_hdr) + (align - 1);
  void  *base    = malloc(totalsz);
  if (base == NULL) return NULL;

  uptr  raw = (uptr)base + sizeof(heap_block_hdr);
  uptr  ptr = align_ptr_up(raw, align);
  usize off = ptr - raw;

  heap_block_hdr* hdr = (heap_block_hdr*)(ptr - sizeof(heap_block_hdr));
  hdr->base             = base;
  hdr->size             = totalsz;
  hdr->align            = align;
  hdr->offset           = off;

  return (void*)ptr;
}


static void *heap_allocate(void *udata, usize sz) {
  return heap_allocate_aligned(udata, sz, alignof(max_align_t));
}


static void *heap_reallocate(void *udata, void *ptr, usize oldsz, usize newsz) {
  (void)udata;
  (void)oldsz;

  if (ptr == NULL) {
    return heap_allocate(udata, newsz);
  }
  else if (newsz == 0) {
    heap_block_hdr *hdr = (heap_block_hdr*)((uptr)ptr - sizeof(heap_block_hdr));
    free(hdr->base);
    return NULL;
  }
  else {
    heap_block_hdr *old_hdr = (heap_block_hdr*)((uptr)ptr - sizeof(heap_block_hdr));

    if (!is_ptr_aligned((uptr)ptr, old_hdr->align)) {
      return NULL;
    }

    usize align = old_hdr->align;
    usize copy  = (oldsz < newsz ? oldsz : newsz);

    usize  new_totalsz = newsz + sizeof(heap_block_hdr) + (align - 1);
    void  *new_base    = realloc(old_hdr->base, new_totalsz);
    if (new_base == NULL) {
      return NULL;
    }

    uptr  new_raw = (uptr)new_base + sizeof(heap_block_hdr);
    uptr  new_ptr = align_ptr_up(new_raw, align);
    usize new_off = new_ptr - new_raw;

    void *old_payload_in_new_block = new_base + sizeof(heap_block_hdr) + old_hdr->offset;
    void *new_payload_location     = (void*)new_ptr;

    if (new_payload_location != old_payload_in_new_block) {
      memmove(new_payload_location, old_payload_in_new_block, copy);
    }

    heap_block_hdr *new_hdr = (heap_block_hdr*)(new_ptr - sizeof(heap_block_hdr));
    new_hdr->base           = new_base;
    new_hdr->size           = new_totalsz;
    new_hdr->align          = align;
    new_hdr->offset         = new_off;

    return (void*)new_ptr;
  }
}


static void heap_deallocate(void *udata, void *ptr, usize sz) {
  (void)udata;
  (void)sz;

  heap_block_hdr *hdr = (heap_block_hdr*)((uptr)ptr - sizeof(heap_block_hdr));
  free(hdr->base);
}


allocator heap_allocator(void) {
  return (allocator) {
    .allocate         = heap_allocate,
    .allocate_aligned = heap_allocate_aligned,
    .reallocate       = heap_reallocate,
    .deallocate       = heap_deallocate,
    .udata            = NULL,
  };
}

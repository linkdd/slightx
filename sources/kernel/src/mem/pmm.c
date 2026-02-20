#include <limine.h>

#include <kernel/mem/pmm.h>
#include <kernel/mem/hhdm.h>

#include <klibc/collections/bitmap.h>
#include <klibc/mem/align.h>
#include <klibc/mem/bytes.h>
#include <klibc/sync/lock.h>
#include <klibc/assert.h>


typedef struct pmm pmm;
struct pmm {
  spinlock lock;
  uptr     highest_page;
  bitmap   allocated_pages;
};


static pmm instance = {};


static void  pmm__find_highest_page(pmm *self);
static usize pmm__get_page_count   (pmm *self);
static void  pmm__reserve_bitmap   (pmm *self);
static void  pmm__scan_bitmap      (pmm *self);
static void  pmm__mark_pages       (pmm *self, u64 page, usize length);
static void  pmm__clear_pages      (pmm *self, u64 page, usize length);
static bool  pmm__is_page_available(uptr page_addr);


void pmm_init(void) {
  spinlock_init(&instance.lock);
  bitmap_init  (&instance.allocated_pages, span_null());
}


void pmm_load(void) {
  LIMINE_GET_RESP(memmap);
  assert_release(memmap_response != NULL);

  pmm__find_highest_page(&instance);
  pmm__reserve_bitmap  (&instance);
  pmm__scan_bitmap     (&instance);
}


physical_address pmm_alloc(usize page_count) {
  spinlock_acquire(&instance.lock);

  usize total_page_count = pmm__get_page_count(&instance);

  for (usize page_base = 0; page_base < total_page_count; page_base++) {
    for (usize page_offset = 0; page_offset < page_count; page_offset++) {
      if (bitmap_isset(&instance.allocated_pages, page_base + page_offset)) {
        break;
      }

      if (page_offset == page_count - 1) {
        pmm__mark_pages(&instance, page_base, page_count);

        spinlock_release(&instance.lock);

        physical_address pa = { .addr = (uptr)(page_base * MM_PHYS_PAGE_SIZE) };
        virtual_address  va = hhdm_p2v(pa);

        va.ptr = memset(va.ptr, 0, page_count * MM_PHYS_PAGE_SIZE);

        return hhdm_v2p(va);
      }
    }
  }

  spinlock_release(&instance.lock);
  panic("[pmm] No free pages available");
}


void pmm_free(physical_address pa, usize page_count) {
  u64 page = pa.addr / MM_PHYS_PAGE_SIZE;
  pmm__clear_pages(&instance, page, page_count);
}



static void pmm__find_highest_page(pmm *self) {
  LIMINE_GET_RESP(memmap);

  for (usize idx = 0; idx < memmap_response->entry_count; idx++) {
    struct limine_memmap_entry *entry = memmap_response->entries[idx];
    uptr page;

    switch (entry->type) {
      case LIMINE_MEMMAP_USABLE:
      case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
      case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
        page = entry->base + entry->length;

        if (page > self->highest_page) {
          self->highest_page = page;
        }

        break;

      default:
        break;
    }
  }
}


static usize pmm__get_page_count(pmm *self) {
  usize top = (usize) align_ptr_down(self->highest_page, MM_PHYS_PAGE_SIZE);
  return top / MM_PHYS_PAGE_SIZE;
}


static void pmm__reserve_bitmap(pmm *self) {
  LIMINE_GET_RESP(memmap);

  usize bitmap_size = align_size_up(pmm__get_page_count(self) / 8, MM_PHYS_PAGE_SIZE);

  for (usize idx = 0; idx < memmap_response->entry_count; idx++) {
    struct limine_memmap_entry *entry = memmap_response->entries[idx];

    if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
      physical_address bitmap_paddr = { .addr = entry->base };
      virtual_address  bitmap_vaddr = hhdm_p2v(bitmap_paddr);

      entry->base   += bitmap_size;
      entry->length -= bitmap_size;

      bitmap_init(&self->allocated_pages, make_span(bitmap_vaddr.ptr, bitmap_size));

      return;
    }
  }

  panic("[pmm] Not enough memory.");
}


static void pmm__scan_bitmap(pmm *self) {
  usize page_count = pmm__get_page_count(self);

  for (usize page = 0; page < page_count; page++) {
    if (!pmm__is_page_available(page * MM_PHYS_PAGE_SIZE)) {
      bitmap_set(&self->allocated_pages, page);
    }
  }
}


static void pmm__mark_pages(pmm *self, u64 page, usize length) {
  for (usize offset = 0; offset < length; offset++) {
    bitmap_set(&self->allocated_pages, page + offset);
  }
}


static void pmm__clear_pages(pmm *self, u64 page, usize length) {
  spinlock_acquire(&self->lock);

  for (usize offset = 0; offset < length; offset++) {
    bitmap_clear(&self->allocated_pages, page + offset);
  }

  spinlock_release(&self->lock);
}


static bool pmm__is_page_available(uptr page_addr) {
  LIMINE_GET_RESP(memmap);

  for (usize idx = 0; idx < memmap_response->entry_count; idx++) {
    struct limine_memmap_entry *entry = memmap_response->entries[idx];

    if (
      page_addr >= entry->base &&
      (page_addr + MM_PHYS_PAGE_SIZE) <= (entry->base + entry->length)
    ) {
      return entry->type == LIMINE_MEMMAP_USABLE;
    }
  }

  return false;
}

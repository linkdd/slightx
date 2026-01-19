#pragma once

#include <klibc/types.h>
#include <klibc/mem/alloc.h>

#include <kernel/mem/addr.h>


#define MM_VIRT_PAGE_SIZE   0x1000 // 4kb

#define PT_SIZE             (0x1000)

#define PAGE_4KB_SIZE       (0x1000)
#define PAGE_2MB_SIZE       (0x200000)
#define PAGE_1GB_SIZE       (0x40000000)

#define PAGE_4KB_ALIGNMENT  (PAGE_4KB_SIZE)
#define PAGE_2MB_ALIGNMENT  (PAGE_2MB_SIZE)
#define PAGE_1GB_ALIGNMENT  (PAGE_1GB_SIZE)

#define PT_FLAG_VALID       ((pt_flags) 1 << 0)
#define PT_FLAG_WRITE       ((pt_flags) 1 << 1)
#define PT_FLAG_USER        ((pt_flags) 1 << 2)
#define PT_FLAG_LARGE       ((pt_flags) 1 << 7)
#define PT_FLAG_GLOBAL      ((pt_flags) 1 << 8)
#define PT_FLAG_NX          ((pt_flags) 1 << 63)
#define PT_PADDR_MASK       ((pt_flags) 0x0000FFFFFFFFF000)

#define PT_KERN_TABLE_FLAGS (PT_FLAG_VALID | PT_FLAG_WRITE)
#define PT_USER_TABLE_FLAGS (PT_FLAG_VALID | PT_FLAG_WRITE | PT_FLAG_USER)
#define PT_TO_VMM_FLAGS(x)  ((x) & (PT_FLAG_WRITE | PT_FLAG_NX))


typedef u64 page_table_entry;
typedef u64 pt_flags;

typedef enum {
  VMM_PML1 = 0,
  VMM_PML2,
  VMM_PML3,
  VMM_PML4,
  VMM_PML5,
} page_map_level;

typedef struct page_map page_map;
struct page_map {
  page_map_level    level;
  page_table_entry *directory;
};

typedef enum {
  VMM_PAGE_SIZE_4KB = 0,
  VMM_PAGE_SIZE_2MB,
  VMM_PAGE_SIZE_1GB,
} page_size;


void vmm_init(void);
void vmm_load(void);

void vmm_map(
  page_map  *pmap,
  allocator  a,

  virtual_address  va,
  physical_address pa,

  pt_flags  flags,
  page_size size
);

void vmm_unmap(page_map *pmap, virtual_address va);

void vmm_switch_page_map(page_map *map);
void vmm_invalidate_page(virtual_address va);

physical_address vmm_translate(page_map *pmap, virtual_address va);

void vmm_make_page_map   (allocator a, page_map *pmap);
void vmm_destroy_page_map(allocator a, page_map *pmap);
void vmm_clone_page_map  (allocator a, page_map *dst, page_map *src);

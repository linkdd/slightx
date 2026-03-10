#pragma once

#include <klibc/types.h>

#include <kernel/mem/addr.h>


#define MM_VIRT_PAGE_SIZE      (0x1000) // 4kb

#define MM_PT_SIZE             (0x1000)

#define MM_PAGE_4KB            (0x1000)
#define MM_PAGE_2MB            (0x200000)
#define MM_PAGE_1GB            (0x40000000)

#define MM_PT_FLAG_VALID       ((pt_flags) 1 << 0)
#define MM_PT_FLAG_WRITE       ((pt_flags) 1 << 1)
#define MM_PT_FLAG_USER        ((pt_flags) 1 << 2)
#define MM_PT_FLAG_LARGE       ((pt_flags) 1 << 7)
#define MM_PT_FLAG_GLOBAL      ((pt_flags) 1 << 8)
#define MM_PT_FLAG_NX          ((pt_flags) 1 << 63)
#define MM_PT_PADDR_MASK       ((pt_flags) 0x0000FFFFFFFFF000)

#define MM_PT_KERN_TABLE_FLAGS (MM_PT_FLAG_VALID | MM_PT_FLAG_WRITE)
#define MM_PT_USER_TABLE_FLAGS (MM_PT_FLAG_VALID | MM_PT_FLAG_WRITE | MM_PT_FLAG_USER)
#define MM_PT_DEFAULT_FLAGS(x) ((x) & (MM_PT_FLAG_WRITE | MM_PT_FLAG_NX))


typedef u64   page_table_entry;
typedef u64   pt_flags;
typedef usize page_size;

typedef enum {
  PML1 = 0,
  PML2,
  PML3,
  PML4,
  PML5,
} page_map_level;

typedef struct page_map page_map;
struct page_map {
  page_map_level    level;
  page_table_entry *directory;
};


void vmm_init(void);
void vmm_load(void);

page_map *vmm_get_kernel_page_map(void);

void vmm_map(
  page_map *pmap,

  virtual_address  va,
  physical_address pa,

  pt_flags  flags,
  page_size size
);

void vmm_unmap(page_map *pmap, virtual_address va);

void vmm_switch_page_map(page_map *map);
void vmm_invalidate_page(virtual_address va);

physical_address vmm_translate(page_map *pmap, virtual_address va);
bool             vmm_is_mapped(page_map *pmap, virtual_address va);

void vmm_make_page_map   (page_map *pmap);
void vmm_destroy_page_map(page_map *pmap);
void vmm_clone_page_map  (page_map *dst, page_map *src);

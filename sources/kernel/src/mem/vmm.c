#include <limine.h>

#include <klibc/io/log.h>
#include <klibc/sync/lock.h>
#include <klibc/assert.h>

#include <kernel/mem/vmm.h>
#include <kernel/mem/pmm.h>
#include <kernel/mem/hhdm.h>
#include <kernel/panic.h>

#include <kernel/boot/exc.h>
#include <kernel/cpu/cpuid.h>

#include <klibc/mem/align.h>
#include <klibc/mem/bytes.h>


static page_map kernel_page_map = {};
static spinlock vmm_lock        = {};


static bool vmm__pages_1gb_support(void) {
  u32 eax, ebx, ecx, edx;

  if (cpuid(0x80000001, 0, &eax, &ebx, &ecx, &edx)) {
    return (edx & (1 << 26)) == (1 << 26);
  }

  return false;
}


// MARK: pml helpers
static inline usize pml5_index(virtual_address va) {
  return (va.addr >> 48) & 0x1FF;
}


static inline usize pml4_index(virtual_address va) {
  return (va.addr >> 39) & 0x1FF;
}


static inline usize pml3_index(virtual_address va) {
  return (va.addr >> 30) & 0x1FF;
}


static inline usize pml2_index(virtual_address va) {
  return (va.addr >> 21) & 0x1FF;
}


static inline usize pml1_index(virtual_address va) {
  return (va.addr >> 12) & 0x1FF;
}


// MARK: pte helpers
static inline bool pte_present(page_table_entry pte) {
  return (pte & MM_PT_FLAG_VALID) != 0;
}


static inline bool pte_large(page_table_entry pte) {
  return (pte & MM_PT_FLAG_LARGE) != 0;
}


static inline page_table_entry *pte_p2v(page_table_entry pte) {
  physical_address pa = { .addr = pte & MM_PT_PADDR_MASK };
  virtual_address  va = hhdm_p2v(pa);
  return (page_table_entry*)va.ptr;
}


// MARK: pd helpers
static inline page_table_entry *pd_split(
  page_table_entry *pd,

  usize     index,
  page_size old_page_size,
  page_size new_page_size,
  pt_flags  table_flags
) {
  page_table_entry pte = pd[index];

  if (!pte_present(pte) || !pte_large(pte)) {
    panic("[vmm] pd_split(): called on non-large page");
  }

  if (!(
    (old_page_size == MM_PAGE_1GB && new_page_size == MM_PAGE_2MB) ||
    (old_page_size == MM_PAGE_2MB && new_page_size == MM_PAGE_4KB)
  )) {
    panic("[vmm] pd_split(): called with invalid page sizes");
  }

  const uptr phys_base  = pte & MM_PT_PADDR_MASK;
  u64        leaf_flags = MM_PT_DEFAULT_FLAGS(pte) | MM_PT_FLAG_VALID;

  if (new_page_size == MM_PAGE_2MB) {
    leaf_flags |= MM_PT_FLAG_LARGE;
  }

  physical_address  child_paddr = pmm_alloc(1);
  virtual_address   child_vaddr = hhdm_p2v(child_paddr);
  page_table_entry *child       = child_vaddr.ptr;

  const usize count = old_page_size / new_page_size;

  for (usize i = 0; i < count; i++) {
    child[i] = (page_table_entry)((phys_base + (i * new_page_size)) | leaf_flags);
  }

  pd[index] = (page_table_entry)(child_paddr.addr | table_flags);

  return child;
}


static inline page_table_entry *pd_get(page_table_entry *pd, usize index) {
  page_table_entry pte = pd[index];
  return (pte_present(pte) && !pte_large(pte)) ? pte_p2v(pte) : NULL;
}


static inline page_table_entry *pd_get_or_create(
  page_table_entry *pd,

  usize          index,
  page_map_level level,
  pt_flags       table_flags
) {
  page_table_entry pte = pd[index];

  if (!pte_present(pte)) {
    physical_address t_paddr = pmm_alloc(1);
    virtual_address  t_vaddr = hhdm_p2v(t_paddr);

    pd[index] = (page_table_entry)(t_paddr.addr | table_flags);
    return t_vaddr.ptr;
  }

  if (!pte_large(pte)) {
    if ((table_flags & MM_PT_FLAG_USER) && !(pte & MM_PT_FLAG_USER)) {
      pd[index] |= MM_PT_FLAG_USER;
    }

    return pte_p2v(pte);
  }

  switch (level) {
    case PML3: return pd_split(pd, index, MM_PAGE_1GB, MM_PAGE_2MB, table_flags);
    case PML2: return pd_split(pd, index, MM_PAGE_2MB, MM_PAGE_4KB, table_flags);
    default:
      panic("[vmm] pd_get_or_create(): invalid level for split");
  }
}


// MARK: PF handler
void pagefault_handler(interrupt_frame *iframe, interrupt_controlflow *cf) {
  *cf = INTERRUPT_CONTROLFLOW_HALT;

  klog("=== PAGE FAULT ===");
  klog(" ADDRESS:   %x", iframe->cr2);
  klog(" LOCATION:  %x", iframe->rip);
  klog(" ERROR:     P=%d WR=%d US=%d RSVD=%d ID=%d PK=%d SS=%d",
    (i64)((iframe->err_code >> 0) & 1),
    (i64)((iframe->err_code >> 1) & 1),
    (i64)((iframe->err_code >> 2) & 1),
    (i64)((iframe->err_code >> 3) & 1),
    (i64)((iframe->err_code >> 4) & 1),
    (i64)((iframe->err_code >> 5) & 1),
    (i64)((iframe->err_code >> 6) & 1)
  );
  klog(" REGISTERS:");
  klog("  CR0[%x]",    iframe->cr0);
  klog("  CR3[%x]",    iframe->cr3);
  klog("  CR4[%x]",    iframe->cr4);
  klog("  RFLAGS[%x]", iframe->rflags);
}


// MARK: init
void vmm_init(void) {
  spinlock_init(&vmm_lock);
}


// MARK: load
void vmm_load(void) {
  LIMINE_GET_RESP(paging_mode);
  assert_release(paging_mode_response != NULL);

  exc_set_callback(EXC_PAGE_FAULT, pagefault_handler);

  uptr cr3;
  __asm__ volatile("mov %%cr3, %0" : "=r"(cr3) :: "memory");

  physical_address pmd_paddr = { .addr = cr3 };
  virtual_address  pmd_vaddr = hhdm_p2v(pmd_paddr);

  kernel_page_map.directory = (page_table_entry*)pmd_vaddr.ptr;
  kernel_page_map.level     = (paging_mode_response->mode == LIMINE_PAGING_MODE_X86_64_5LVL
    ? PML5
    : PML4
  );
}

// MARK: accessor
page_map *vmm_get_kernel_page_map(void) {
  return &kernel_page_map;
}

// MARK: map
static void vmm_map_impl(
  page_map *pmap,

  virtual_address  va,
  physical_address pa,

  pt_flags  flags,
  page_size size
) {
  assert(
    size == MM_PAGE_4KB ||
    size == MM_PAGE_2MB ||
    size == MM_PAGE_1GB
  );

  assert_release(
    is_ptr_aligned(va.addr, size) &&
    is_ptr_aligned(pa.addr, size)
  );

  flags                      |= MM_PT_FLAG_VALID;
  const pt_flags table_flags  = (flags & MM_PT_FLAG_USER) ? MM_PT_USER_TABLE_FLAGS : MM_PT_KERN_TABLE_FLAGS;

  page_table_entry *pml4 = NULL;
  switch (pmap->level) {
    case PML5: {
      page_table_entry* pml5 = pmap->directory;

      pml4 = pd_get_or_create(pml5, pml5_index(va), PML5, table_flags);
      break;
    }

    case PML4: {
      pml4 = pmap->directory;
      break;
    }

    default:
      panic("[vmm] vmm_map(): invalid page map level");
  }

  page_table_entry *pml3 = pd_get_or_create(pml4, pml4_index(va), PML4, table_flags);
  if (size == MM_PAGE_1GB) {
    if (vmm__pages_1gb_support()) {
      pml3[pml3_index(va)] = (page_table_entry)(pa.addr | flags | MM_PT_FLAG_LARGE);
    }
    else {
      for (isize offset = 0; offset < MM_PAGE_1GB; offset += MM_PAGE_2MB) {
        vmm_map_impl(
          pmap,
          (virtual_address) {.addr = va.addr + offset},
          (physical_address){.addr = pa.addr + offset},
          flags,
          MM_PAGE_2MB
        );
      }
    }
  }

  page_table_entry *pml2 = pd_get_or_create(pml3, pml3_index(va), PML3, table_flags);
  if (size != MM_PAGE_4KB) {
    pml2[pml2_index(va)] = (page_table_entry)(pa.addr | flags | MM_PT_FLAG_LARGE);
    return;
  }

  page_table_entry *pml1 = pd_get_or_create(pml2, pml2_index(va), PML2, table_flags);
  pml1[pml1_index(va)] = (page_table_entry)(pa.addr | flags);
}


void vmm_map(
  page_map *pmap,

  virtual_address  va,
  physical_address pa,

  pt_flags  flags,
  page_size size
) {
  if (pmap == &kernel_page_map) spinlock_acquire(&vmm_lock);
  vmm_map_impl(pmap, va, pa, flags, size);
  if (pmap == &kernel_page_map) spinlock_release(&vmm_lock);
}


// MARK: unmap
static void vmm_unmap_impl(page_map *pmap, virtual_address va) {
  page_table_entry *pml4 = NULL;

  switch (pmap->level) {
    case PML5: {
      page_table_entry *pml5 = pmap->directory;
      page_table_entry  e    = pml5[pml5_index(va)];

      if (!pte_present(e)) return;
      if (pte_large(e)) {
        panic("[vmm] vmm_unmap(): large pages not supported at PML5 level");
      }

      pml4 = pd_get(pml5, pml5_index(va));
      break;
    }

    case PML4:
      pml4 = pmap->directory;
      break;

    default:
      panic("[vmm] vmm_unmap(): invalid page map level");
  }

  page_table_entry e;

  e = pml4[pml4_index(va)];
  if (!pte_present(e)) return;
  if (pte_large(e)) {
    panic("[vmm] vmm_unmap(): large pages not supported at PML4 level");
  }

  page_table_entry *pml3 = pd_get(pml4, pml4_index(va));
  e = pml3[pml3_index(va)];
  if (!pte_present(e)) return;
  if (pte_large(e)) {
    pml3[pml3_index(va)] = 0;
    vmm_invalidate_page(va);
    return;
  }

  page_table_entry *pml2 = pd_get(pml3, pml3_index(va));
  e = pml2[pml2_index(va)];
  if (!pte_present(e)) return;
  if (pte_large(e)) {
    pml2[pml2_index(va)] = 0;
    vmm_invalidate_page(va);
    return;
  }

  page_table_entry *pml1 = pd_get(pml2, pml2_index(va));
  e = pml1[pml1_index(va)];
  if (!pte_present(e)) return;
  pml1[pml1_index(va)] = 0;
  vmm_invalidate_page(va);
}


void vmm_unmap(page_map *pmap, virtual_address va) {
  if (pmap == &kernel_page_map) spinlock_acquire(&vmm_lock);
  vmm_unmap_impl(pmap, va);
  if (pmap == &kernel_page_map) spinlock_release(&vmm_lock);
}


// MARK: switch
void vmm_switch_page_map(page_map *pmap) {
  virtual_address  va = { .ptr = pmap->directory };
  physical_address pa = hhdm_v2p(va);

  __asm__ volatile("mov %0, %%cr3" : : "r"(pa.ptr));
}


// MARK: invalidate
void vmm_invalidate_page(virtual_address va) {
  __asm__ volatile(
    "invlpg (%0)"
    : : "b"(va.addr)
    : "memory"
  );
}


// MARK: translate
static physical_address vmm_translate_impl(page_map *pmap, virtual_address va) {
  page_table_entry *pml4 = NULL;
  page_table_entry  e;

  switch (pmap->level) {
    case PML5: {
      page_table_entry *pml5 = pmap->directory;
      e = pml5[pml5_index(va)];
      if (!pte_present(e)) {
        panic("[vmm] vmm_translate(): invalid address at PML5 level");
      }
      if (pte_large(e)) {
        return (physical_address) { .addr = (e & MM_PT_PADDR_MASK) | (va.addr & (MM_PAGE_1GB - 1)) };
      }
      pml4 = pte_p2v(e);
      break;
    }

    case PML4:
      pml4 = pmap->directory;
      break;

    default:
      panic("[vmm] vmm_translate(): invalid page map level");
  }

  e = pml4[pml4_index(va)];
  if (!pte_present(e)) {
    panic("[vmm] vmm_translate(): invalid address at PML4 level");
  }
  if (pte_large(e)) {
    return (physical_address) { .addr = (e & MM_PT_PADDR_MASK) | (va.addr & (MM_PAGE_1GB - 1)) };
  }

  page_table_entry *pml3 = pte_p2v(e);
  e = pml3[pml3_index(va)];
  if (!pte_present(e)) {
    panic("[vmm] vmm_translate(): invalid address at PML3 level");
  }
  if (pte_large(e)) {
    return (physical_address) { .addr = (e & MM_PT_PADDR_MASK) | (va.addr & (MM_PAGE_2MB - 1)) };
  }

  page_table_entry *pml2 = pte_p2v(e);
  e = pml2[pml2_index(va)];
  if (!pte_present(e)) {
    panic("[vmm] vmm_translate(): invalid address at PML2 level");
  }
  if (pte_large(e)) {
    return (physical_address) { .addr = (e & MM_PT_PADDR_MASK) | (va.addr & (MM_PAGE_2MB - 1)) };
  }

  page_table_entry *pml1 = pte_p2v(e);
  e = pml1[pml1_index(va)];
  if (!pte_present(e)) {
    panic("[vmm] vmm_translate(): invalid address at PML1 level");
  }

  return (physical_address) { .addr = (e & MM_PT_PADDR_MASK) | (va.addr & (MM_PAGE_4KB - 1)) };
}


physical_address vmm_translate(page_map *pmap, virtual_address va) {
  if (pmap == &kernel_page_map) spinlock_acquire(&vmm_lock);
  physical_address pa = vmm_translate_impl(pmap, va);
  if (pmap == &kernel_page_map) spinlock_release(&vmm_lock);
  return pa;
}



// MARK: constructor
void vmm_make_page_map(page_map *pmap) {
  LIMINE_GET_RESP(paging_mode);
  assert_release(paging_mode_response != NULL);

  if (paging_mode_response->mode == LIMINE_PAGING_MODE_X86_64_5LVL) {
    pmap->level = PML5;
  }
  else {
    pmap->level = PML4;
  }

  physical_address pa = pmm_alloc(1);
  virtual_address  va = hhdm_p2v(pa);

  pmap->directory = va.ptr;

  // Copy kernel upper-half entries so the kernel is mapped in every address space.
  if (pmap->level == PML4) {
    for (usize i = 256; i < 512; i++) {
      pmap->directory[i] = kernel_page_map.directory[i];
    }
  }
  else {
    for (usize i = 256; i < 512; i++) {
      pmap->directory[i] = kernel_page_map.directory[i];
    }
  }
}


// MARK: destroy
static void vmm_destroy_tables(page_table_entry *tbl, page_map_level level) {
  if (tbl == NULL)   return;
  if (level == PML1) return;

  // At the PML4 (or PML5) level, only iterate user-space entries (lower half,
  // indices 0-255). Entries 256-511 are shared kernel mappings copied from the
  // kernel page map and must never be freed. Note: Limine sets the USER flag on
  // ALL intermediate page table entries (PT_TABLE_FLAGS includes USER), so the
  // USER-flag check alone is insufficient to distinguish user-private vs.
  // kernel-shared entries at this level.
  usize entry_limit = 512;
  if (level == PML4 || level == PML5) {
    entry_limit = 256;
  }

  for (usize i = 0; i < entry_limit; i++) {
    page_table_entry pte = tbl[i];

    if (!pte_present(pte))         continue;
    if (pte_large(pte))            continue;
    if ((pte & MM_PT_FLAG_USER) == 0) continue;

    page_table_entry *child = pte_p2v(pte);

    page_map_level next_level;
    switch (level) {
      case PML5: next_level = PML4; break;
      case PML4: next_level = PML3; break;
      case PML3: next_level = PML2; break;
      case PML2: next_level = PML1; break;
      case PML1:
      default:
        panic("[vmm] vmm_destroy_tables(): invalid page map level");
    }

    if (next_level != PML1) {
      vmm_destroy_tables(child, next_level);
    }

    virtual_address  child_vaddr = { .ptr = child };
    physical_address child_paddr = hhdm_v2p(child_vaddr);

    pmm_free(child_paddr, 1);
    tbl[i] = 0;
  }
}

void vmm_destroy_page_map(page_map *pmap) {
  if (pmap == NULL || pmap->directory == NULL) {
    return;
  }

  if (pmap == &kernel_page_map) {
    panic("[vmm] vmm_destroy_page_map(): attempt to destroy kernel page map");
  }

  vmm_destroy_tables(pmap->directory, pmap->level);

  virtual_address  va = { .ptr = pmap->directory };
  physical_address pa = hhdm_v2p(va);

  pmm_free(pa, 1);
  pmap->directory = NULL;
}


// MARK: clone
static void vmm_clone_tables(page_table_entry *dst_tbl, page_table_entry *src_tbl, page_map_level level) {
  const pt_flags user_table_flags = MM_PT_USER_TABLE_FLAGS;

  for (usize i = 0; i < 512; i++) {
    page_table_entry src_pte = src_tbl[i];
    if (!pte_present(src_pte)) {
      continue;
    }

    if ((src_pte & MM_PT_FLAG_USER) == 0) {
      dst_tbl[i] = src_pte;
      continue;
    }

    if (pte_large(src_pte)) {
      dst_tbl[i] = src_pte;
      continue;
    }

    physical_address  child_paddr = pmm_alloc(1);
    virtual_address   child_vaddr = hhdm_p2v(child_paddr);
    page_table_entry *child       = child_vaddr.ptr;

    dst_tbl[i] = (page_table_entry)(child_paddr.addr | user_table_flags);

    page_table_entry *src_child = pte_p2v(src_pte);

    page_map_level next_level;
    switch (level) {
      case PML5: next_level = PML4; break;
      case PML4: next_level = PML3; break;
      case PML3: next_level = PML2; break;
      case PML2: next_level = PML1; break;
      case PML1:
      default:
        panic("[vmm] vmm_clone_tables(): invalid page map level");
    }

    vmm_clone_tables(child, src_child, next_level);
  }
}


void vmm_clone_page_map(page_map *dst, page_map *src) {
  dst->level = src->level;

  physical_address pa = pmm_alloc(1);
  virtual_address  va = hhdm_p2v(pa);

  dst->directory = va.ptr;

  return vmm_clone_tables(dst->directory, src->directory, dst->level);
}

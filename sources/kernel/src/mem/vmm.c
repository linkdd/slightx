#include <limine.h>

#include <klibc/io/log.h>
#include <klibc/sync/lock.h>
#include <klibc/assert.h>

#include <kernel/mem/vmm.h>
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
  return (pte & PT_FLAG_VALID) != 0;
}


static inline bool pte_large(page_table_entry pte) {
  return (pte & PT_FLAG_LARGE) != 0;
}


static inline page_table_entry *pte_p2v(page_table_entry pte) {
  physical_address pa = { .addr = pte & PT_PADDR_MASK };
  virtual_address  va = hhdm_p2v(pa);
  return (page_table_entry*)va.ptr;
}


// MARK: pd helpers
static inline page_table_entry *pd_split(
  page_table_entry *pd,
  allocator         a,

  usize    index,
  usize    old_page_size,
  usize    new_page_size,
  pt_flags table_flags
) {
  page_table_entry pte = pd[index];

  if (!pte_present(pte) || !pte_large(pte)) {
    panic("[vmm] pd_split(): called on non-large page");
  }

  if (!(
    (old_page_size == PAGE_1GB_SIZE && new_page_size == PAGE_2MB_SIZE) ||
    (old_page_size == PAGE_2MB_SIZE && new_page_size == PAGE_4KB_SIZE)
  )) {
    panic("[vmm] pd_split(): called with invalid page sizes");
  }

  const uptr phys_base  = pte & PT_PADDR_MASK;
  u64        leaf_flags = PT_TO_VMM_FLAGS(pte) | PT_FLAG_VALID;

  if (new_page_size == PAGE_2MB_SIZE) {
    leaf_flags |= PT_FLAG_LARGE;
  }

  page_table_entry *child = allocate_aligned(a, PT_SIZE, PT_SIZE);

  const usize count = old_page_size / new_page_size;

  for (usize i = 0; i < count; i++) {
    child[i] = (page_table_entry)((phys_base + (i * new_page_size)) | leaf_flags);
  }

  virtual_address  child_vaddr = { .ptr = child };
  physical_address child_paddr = hhdm_v2p(child_vaddr);

  pd[index] = (page_table_entry)(child_paddr.addr | table_flags);

  return child;
}


static inline page_table_entry *pd_get(page_table_entry *pd, usize index) {
  page_table_entry pte = pd[index];
  return (pte_present(pte) && !pte_large(pte)) ? pte_p2v(pte) : NULL;
}


static inline page_table_entry *pd_get_or_create(
  page_table_entry *pd,
  allocator         a,

  usize          index,
  page_map_level level,
  pt_flags       table_flags
) {
  page_table_entry pte = pd[index];

  if (!pte_present(pte)) {
    page_table_entry *t = allocate_aligned(a, PT_SIZE, PT_SIZE);

    virtual_address  t_vaddr = { .ptr = t };
    physical_address t_paddr = hhdm_v2p(t_vaddr);

    pd[index] = (page_table_entry)(t_paddr.addr | table_flags);
    return t;
  }

  if (!pte_large(pte)) {
    if ((table_flags & PT_FLAG_USER) && !(pte & PT_FLAG_USER)) {
      pd[index] |= PT_FLAG_USER;
    }

    return pte_p2v(pte);
  }

  switch (level) {
    case VMM_PML3: return pd_split(pd, a, index, PAGE_1GB_SIZE, PAGE_2MB_SIZE, table_flags);
    case VMM_PML2: return pd_split(pd, a, index, PAGE_2MB_SIZE, PAGE_4KB_SIZE, table_flags);
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
  asm volatile("mov %%cr3, %0" : "=r"(cr3) :: "memory");

  physical_address pmd_paddr = { .addr = cr3 };
  virtual_address  pmd_vaddr = hhdm_p2v(pmd_paddr);

  kernel_page_map.directory = (page_table_entry*)pmd_vaddr.ptr;
  kernel_page_map.level     = (paging_mode_response->mode == LIMINE_PAGING_MODE_X86_64_5LVL
    ? VMM_PML5
    : VMM_PML4
  );
}


// MARK: map
static void vmm_map_impl(
  page_map  *pmap,
  allocator  a,

  virtual_address  va,
  physical_address pa,

  pt_flags   flags,
  page_size  size
) {
  const usize alignment = (
    size == VMM_PAGE_SIZE_1GB ? PAGE_1GB_ALIGNMENT :
    size == VMM_PAGE_SIZE_2MB ? PAGE_2MB_ALIGNMENT :
    /* else */                  PAGE_4KB_ALIGNMENT
  );

  assert_release(
    is_ptr_aligned(va.addr, alignment) &&
    is_ptr_aligned(pa.addr, alignment)
  );

  flags                      |= PT_FLAG_VALID;
  const pt_flags table_flags  = (flags & PT_FLAG_USER) ? PT_USER_TABLE_FLAGS : PT_KERN_TABLE_FLAGS;

  page_table_entry *pml4 = NULL;
  switch (pmap->level) {
    case VMM_PML5: {
      page_table_entry* pml5 = pmap->directory;

      pml4 = pd_get_or_create(pml5, a, pml5_index(va), VMM_PML5, table_flags);
      break;
    }

    case VMM_PML4: {
      pml4 = pmap->directory;
      break;
    }

    default:
      panic("[vmm] vmm_map(): invalid page map level");
  }

  page_table_entry *pml3 = pd_get_or_create(pml4, a, pml4_index(va), VMM_PML4, table_flags);
  if (size == VMM_PAGE_SIZE_1GB) {
    if (vmm__pages_1gb_support()) {
      pml3[pml3_index(va)] = (page_table_entry)(pa.addr | flags | PT_FLAG_LARGE);
    }
    else {
      for (isize offset = 0; offset < PAGE_1GB_SIZE; offset += PAGE_2MB_SIZE) {
        vmm_map_impl(
          pmap,
          a,
          (virtual_address) {.addr = va.addr + offset},
          (physical_address){.addr = pa.addr + offset},
          flags,
          VMM_PAGE_SIZE_2MB
        );
      }
    }
  }

  page_table_entry *pml2 = pd_get_or_create(pml3, a, pml3_index(va), VMM_PML3, table_flags);
  if (size != VMM_PAGE_SIZE_4KB) {
    pml2[pml2_index(va)] = (page_table_entry)(pa.addr | flags | PT_FLAG_LARGE);
    return;
  }

  page_table_entry *pml1 = pd_get_or_create(pml2, a, pml2_index(va), VMM_PML2, table_flags);
  pml1[pml1_index(va)] = (page_table_entry)(pa.addr | flags);
}


void vmm_map(
  page_map  *pmap,
  allocator  a,

  virtual_address  va,
  physical_address pa,

  pt_flags  flags,
  page_size size
) {
  if (pmap == &kernel_page_map) spinlock_acquire(&vmm_lock);
  vmm_map_impl(pmap, a, va, pa, flags, size);
  if (pmap == &kernel_page_map) spinlock_release(&vmm_lock);
}


// MARK: unmap
static void vmm_unmap_impl(page_map *pmap, virtual_address va) {
  page_table_entry *pml4 = NULL;

  switch (pmap->level) {
    case VMM_PML5: {
      page_table_entry *pml5 = pmap->directory;
      page_table_entry  e    = pml5[pml5_index(va)];

      if (!pte_present(e)) return;
      if (pte_large(e)) {
        panic("[vmm] vmm_unmap(): large pages not supported at PML5 level");
      }

      pml4 = pd_get(pml5, pml5_index(va));
      break;
    }

    case VMM_PML4:
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

  asm volatile("mov %0, %%cr3" : : "r"(pa.ptr));
}


// MARK: invalidate
void vmm_invalidate_page(virtual_address va) {
  asm volatile(
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
    case VMM_PML5: {
      page_table_entry *pml5 = pmap->directory;
      e = pml5[pml5_index(va)];
      if (!pte_present(e)) {
        panic("[vmm] vmm_translate(): invalid address at PML5 level");
      }
      if (pte_large(e)) {
        return (physical_address) { .addr = (e & PT_PADDR_MASK) | (va.addr & (PAGE_1GB_SIZE - 1)) };
      }
      pml4 = pte_p2v(e);
      break;
    }

    case VMM_PML4:
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
    return (physical_address) { .addr = (e & PT_PADDR_MASK) | (va.addr & (PAGE_1GB_SIZE - 1)) };
  }

  page_table_entry *pml3 = pte_p2v(e);
  e = pml3[pml3_index(va)];
  if (!pte_present(e)) {
    panic("[vmm] vmm_translate(): invalid address at PML3 level");
  }
  if (pte_large(e)) {
    return (physical_address) { .addr = (e & PT_PADDR_MASK) | (va.addr & (PAGE_2MB_SIZE - 1)) };
  }

  page_table_entry *pml2 = pte_p2v(e);
  e = pml2[pml2_index(va)];
  if (!pte_present(e)) {
    panic("[vmm] vmm_translate(): invalid address at PML2 level");
  }
  if (pte_large(e)) {
    return (physical_address) { .addr = (e & PT_PADDR_MASK) | (va.addr & (PAGE_2MB_SIZE - 1)) };
  }

  page_table_entry *pml1 = pte_p2v(e);
  e = pml1[pml1_index(va)];
  if (!pte_present(e)) {
    panic("[vmm] vmm_translate(): invalid address at PML1 level");
  }

  return (physical_address) { .addr = (e & PT_PADDR_MASK) | (va.addr & (PAGE_4KB_SIZE - 1)) };
}


physical_address vmm_translate(page_map *pmap, virtual_address va) {
  if (pmap == &kernel_page_map) spinlock_acquire(&vmm_lock);
  physical_address pa = vmm_translate_impl(pmap, va);
  if (pmap == &kernel_page_map) spinlock_release(&vmm_lock);
  return pa;
}



// MARK: constructor
void vmm_make_page_map(allocator a, page_map *pmap) {
  LIMINE_GET_RESP(paging_mode);
  assert_release(paging_mode_response != NULL);

  if (paging_mode_response->mode == LIMINE_PAGING_MODE_X86_64_5LVL) {
    pmap->level = VMM_PML5;
  }
  else {
    pmap->level = VMM_PML4;
  }

  pmap->directory = allocate_aligned(a, PT_SIZE, PT_SIZE);
}


// MARK: destroy
static void vmm_destroy_tables(allocator a, page_table_entry *tbl, page_map_level level) {
  if (tbl == NULL)       return;
  if (level == VMM_PML1) return;

  for (usize i = 0; i < 512; i++) {
    page_table_entry pte = tbl[i];

    if (!pte_present(pte))         continue;
    if (pte_large(pte))            continue;
    if ((pte & PT_FLAG_USER) == 0) continue;

    page_table_entry *child = pte_p2v(pte);

    page_map_level next_level;
    switch (level) {
      case VMM_PML5: next_level = VMM_PML4; break;
      case VMM_PML4: next_level = VMM_PML3; break;
      case VMM_PML3: next_level = VMM_PML2; break;
      case VMM_PML2: next_level = VMM_PML1; break;
      case VMM_PML1:
      default:
        panic("[vmm] vmm_destroy_tables(): invalid page map level");
    }

    if (next_level != VMM_PML1) {
      vmm_destroy_tables(a, child, next_level);
    }

    deallocate(a, child, PT_SIZE);
    tbl[i] = 0;
  }
}

void vmm_destroy_page_map(allocator a, page_map *pmap) {
  if (pmap == NULL || pmap->directory == NULL) {
    return;
  }

  if (pmap == &kernel_page_map) {
    panic("[vmm] vmm_destroy_page_map(): attempt to destroy kernel page map");
  }

  vmm_destroy_tables(a, pmap->directory, pmap->level);

  deallocate(a, pmap->directory, PT_SIZE);
  pmap->directory = NULL;
}


// MARK: clone
static void vmm_clone_tables(allocator a, page_table_entry *dst_tbl, page_table_entry *src_tbl, page_map_level level) {
  const pt_flags user_table_flags = PT_USER_TABLE_FLAGS;

  for (usize i = 0; i < 512; i++) {
    page_table_entry src_pte = src_tbl[i];
    if (!pte_present(src_pte)) {
      continue;
    }

    if ((src_pte & PT_FLAG_USER) == 0) {
      dst_tbl[i] = src_pte;
      continue;
    }

    if (pte_large(src_pte)) {
      dst_tbl[i] = src_pte;
      continue;
    }

    page_table_entry *child = allocate_aligned(a, PT_SIZE, PT_SIZE);
    memset(child, 0, PT_SIZE);

    virtual_address  child_vaddr = { .ptr = child };
    physical_address child_paddr = hhdm_v2p(child_vaddr);

    dst_tbl[i] = (page_table_entry)(child_paddr.addr | user_table_flags);

    page_table_entry *src_child = pte_p2v(src_pte);

    page_map_level next_level;
    switch (level) {
      case VMM_PML5: next_level = VMM_PML4; break;
      case VMM_PML4: next_level = VMM_PML3; break;
      case VMM_PML3: next_level = VMM_PML2; break;
      case VMM_PML2: next_level = VMM_PML1; break;
      case VMM_PML1:
      default:
        panic("[vmm] vmm_clone_tables(): invalid page map level");
    }

    vmm_clone_tables(a, child, src_child, next_level);
  }
}


void vmm_clone_page_map(allocator a, page_map *dst, page_map *src) {
  dst->level     = src->level;
  dst->directory = allocate_aligned(a, PT_SIZE, PT_SIZE);
  memset(dst->directory, 0, PT_SIZE);

  return vmm_clone_tables(a, dst->directory, src->directory, dst->level);
}

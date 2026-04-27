#include <klibc/assert.h>

#include <kernel/boot/madt.h>
#include <kernel/mem/hhdm.h>
#include <kernel/panic.h>


static volatile u32 *madt_lapic_mmio;


void madt_mmio_load(void) {
  const madt *m = (const madt*)acpi_find(ACPI_SIG_MADT);
  if (m == NULL) {
    panic("[acpi] could not find MADT");
  }

  uptr      base = (uptr)m->lapic_addr;
  const u8 *ptr  = m->entries;
  const u8 *end  = ((const u8*)m) + m->header.length;

  if (end < ptr) {
    panic("[acpi] MADT length %d truncates entries", (i64)m->header.length);
  }

  while (ptr + sizeof(madt_hdr) <= end) {
    const madt_hdr *entry = (const madt_hdr*)ptr;

    if (entry->len < sizeof(madt_hdr)) {
      panic(
        "[acpi] MADT entry type %d has invalid len %d",
        (i64)entry->type,
        (i64)entry->len
      );
    }
    if ((usize)(end - ptr) < entry->len) {
      panic("[acpi] MADT entry type %d overruns table", (i64)entry->type);
    }

    if (entry->type == MADT_LAPIC_OVERRIDE && entry->len >= sizeof(madt_lapic_override)) {
      const madt_lapic_override *override = (const madt_lapic_override*)entry;
      base = override->addr;
    }

    ptr += entry->len;
  }

  if (base == 0) {
    panic("[acpi] MADT advertised LAPIC base of 0");
  }

  physical_address pa = { .addr = base };
  virtual_address  va = hhdm_p2v(pa);

  madt_lapic_mmio = (volatile u32*)va.ptr;
}


volatile u32 *madt_lapic_base(void) {
  assert(madt_lapic_mmio != NULL);
  return madt_lapic_mmio;
}

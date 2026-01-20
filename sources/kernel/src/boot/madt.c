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

  while (ptr + sizeof(madt_hdr) <= end) {
    const madt_hdr *entry = (const madt_hdr*)ptr;
    if (entry->len == 0) {
      break;
    }

    if (entry->type == MADT_LAPIC_OVERRIDE && entry->len >= sizeof(madt_lapic_override)) {
      const madt_lapic_override *override = (const madt_lapic_override*)entry;
      base = override->addr;
    }

    ptr += entry->len;
  }

  physical_address pa = { .addr = base };
  virtual_address  va = hhdm_p2v(pa);

  madt_lapic_mmio = (volatile u32*)va.ptr;
}


volatile u32 *madt_lapic_base(void) {
  return madt_lapic_mmio;
}

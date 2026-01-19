#include <limine.h>

#include <klibc/assert.h>

#include <kernel/boot/acpi.h>
#include <kernel/mem/hhdm.h>
#include <kernel/panic.h>


static const acpi_sdt_hdr *sdt = NULL;


static u32 acpi_signature(const char sig[4]) {
  return ACPI_SIG32(sig[0], sig[1], sig[2], sig[3]);
}


static bool acpi_verify_checksum(const void *buffer, usize len) {
  const u8 *bytes = (const u8*)buffer;

  u8 sum = 0;
  for (usize i = 0; i < len; i++) {
    sum += bytes[i];
  }

  return sum == 0;
}


static usize acpi_count_entries(void) {
  const acpi_rsdt *rsdt = acpi_get_rsdt();
  const acpi_xsdt *xsdt = acpi_get_xsdt();

  if      (rsdt != NULL) return (rsdt->header.length - sizeof(acpi_sdt_hdr)) / sizeof(u32);
  else if (xsdt != NULL) return (xsdt->header.length - sizeof(acpi_sdt_hdr)) / sizeof(u64);
  else                   return 0;
}


static const acpi_sdt_hdr *acpi_get_entry(usize index) {
  if (index < acpi_count_entries()) {
    const acpi_rsdt *rsdt = acpi_get_rsdt();
    const acpi_xsdt *xsdt = acpi_get_xsdt();

    physical_address pa = {};

    if      (rsdt != NULL) pa.addr = rsdt->entries[index];
    else if (xsdt != NULL) pa.addr = xsdt->entries[index];

    virtual_address va = hhdm_p2v(pa);

    return (const acpi_sdt_hdr*)va.ptr;
  }

  return NULL;
}


void acpi_load(void) {
  LIMINE_GET_RESP(rsdp);
  assert_release(rsdp_response != NULL);

  const acpi_rsdp *rsdp = (const acpi_rsdp*)rsdp_response->address;

  if (
    (rsdp->revision <  2 && !acpi_verify_checksum(rsdp, 20)) ||
    (rsdp->revision >= 2 && !acpi_verify_checksum(rsdp, rsdp->length))
  ) {
    panic("[acpi] invalid RSDP checksum");
  }

  physical_address sdt_addr = {
    .addr = ((rsdp->revision >= 2 && rsdp->xsdt_addr != 0)
      ? rsdp->xsdt_addr
      : rsdp->rsdt_addr
    )
  };

  const acpi_sdt_hdr *hdr = (const acpi_sdt_hdr*)hhdm_p2v(sdt_addr).ptr;

  u32 signature = acpi_signature(hdr->signature);
  if (signature != ACPI_SIG_RSDT && signature != ACPI_SIG_XSDT) {
    panic("[acpi] invalid RSDT/XSDT signature");
  }

  if (!acpi_verify_checksum(hdr, hdr->length)) {
    panic("[acpi] invalid RSDT/XSDT checksum");
  }

  sdt = hdr;
}


const acpi_rsdt *acpi_get_rsdt(void) {
  LIMINE_GET_RESP(rsdp);

  if (rsdp_response->revision < 2) {
    return (const acpi_rsdt*)sdt;
  }

  return NULL;
}


const acpi_xsdt *acpi_get_xsdt(void) {
  LIMINE_GET_RESP(rsdp);

  if (rsdp_response->revision >= 2) {
    return (const acpi_xsdt*)sdt;
  }

  return NULL;
}


const acpi_sdt_hdr *acpi_find(u32 signature) {
  for (usize i = 0; i < acpi_count_entries(); i++) {
    const acpi_sdt_hdr *entry = acpi_get_entry(i);

    if      (entry == NULL)                                 continue;
    else if (signature != acpi_signature(entry->signature)) continue;
    else if (!acpi_verify_checksum(entry, entry->length))   continue;

    return entry;
  }

  return NULL;
}

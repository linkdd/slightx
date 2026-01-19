#pragma once

#include <klibc/types.h>


#define ACPI_SIG32(a,b,c,d) ((u32)(a) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))
#define ACPI_SIG_RSDT       ACPI_SIG32('R','S','D','T')
#define ACPI_SIG_XSDT       ACPI_SIG32('X','S','D','T')


typedef struct acpi_rsdp acpi_rsdp;
struct acpi_rsdp {
  char signature[8];   // "RSD PTR "
  u8   checksum;
  char oemid[6];
  u8   revision;       // 0=ACPI 1.0, 2=ACPI 2.0+
  u32  rsdt_addr;
  u32  length;         // Only in ACPI 2.0+
  u64  xsdt_addr;      // Only in ACPI 2.0+
  u8   ext_checksum;   // Only in ACPI 2.0+
  u8   reserved[3];
} __attribute__((packed));


typedef struct acpi_sdt_hdr acpi_sdt_hdr;
struct acpi_sdt_hdr {
  char signature[4];
  u32  length;
  u8   revision;
  u8   checksum;
  char oemid[6];
  char oem_table_id[8];
  u32  oem_revision;
  u32  creator_id;
  u32  creator_revision;
} __attribute__((packed));

typedef struct acpi_rsdt acpi_rsdt;
struct acpi_rsdt {
  acpi_sdt_hdr header;
  u32          entries[];
} __attribute__((packed));

typedef struct acpi_xsdt acpi_xsdt;
struct acpi_xsdt {
  acpi_sdt_hdr header;
  u64          entries[];
} __attribute__((packed));


void acpi_load(void);

const acpi_rsdt    *acpi_get_rsdt(void);
const acpi_xsdt    *acpi_get_xsdt(void);
const acpi_sdt_hdr *acpi_find    (u32 signature);

#pragma once

#include <klibc/types.h>

#include <kernel/boot/acpi.h>


#define ACPI_SIG_MADT           ACPI_SIG32('A', 'P', 'I', 'C')

#define MADT_PROC_LAPIC         0
#define MADT_IOAPIC             1
#define MADT_IOAPIC_IRQ_SRC     2
#define MADT_LAPIC_NMI          4
#define MADT_LAPIC_OVERRIDE     5
#define MADT_LAPIC_X2APIC       9


typedef struct madt madt;
struct madt {
  acpi_sdt_hdr header;
  u32          lapic_addr;
  u32          flags;
  u8           entries[];
} __attribute__((packed));

typedef struct madt_hdr madt_hdr;
struct madt_hdr {
  u8 type;
  u8 len;
} __attribute__((packed));

typedef struct madt_lapic_override madt_lapic_override;
struct madt_lapic_override {
  madt_hdr hdr;
  u16      reserved;
  u64      addr;
} __attribute__((packed));


void          madt_mmio_load (void);
volatile u32 *madt_lapic_base(void);

#include <mem/gdt.h>
#include <mem/tss.h>
#include <util.h>

struct gdt_entry_t gdt_entries[6];
struct gdt_t p_gdt;
struct tss_entry_t tss_entry;

static void gdt_set_gate (int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

static void write_tss (int num, uint16_t ss0, uint32_t esp0)
{
    /* Compute base and limit of our entry into the GDT */
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof (tss_entry);

    /* add the TSS descriptor's address to the GDT */
    gdt_set_gate (num, base, limit, 0xE9, 0x00);
    /* Init to 0 */
    memset (&tss_entry, 0, sizeof (tss_entry));

    tss_entry.ss0  = ss0;   /* Set the kernel stack segment */
    tss_entry.esp0 = esp0;  /* Set the kernel stack pointer */

    /* Here we set the cs, ss, ds, es, fs and gs entries in the TSS. These specify what
     * segments should be loaded when the processor switches to kernel mode. Therefore
     * they are just our normal kernel code/data segments - 0x08 and 0x10 respectively,
     * but with the last two bits set, making 0x0b and 0x13. The setting of these bits
     * sets the RPL (requested privilege level) to 3, meaning that this TSS can be used
     * to switch to kernel mode from ring 3.
     */
    tss_entry.cs = 0x0B;
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
}

void set_kernel_stack (uint32_t stack)
{
    tss_entry.esp0 = stack;
}

void init_gdt (void)
{
    p_gdt.limit = (sizeof (struct gdt_entry_t) * 6) - 1;
    p_gdt.base  = (uint32_t) &gdt_entries;

    gdt_set_gate (0, 0, 0, 0, 0);                /* Null segment */
    gdt_set_gate (1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Code segment */
    gdt_set_gate (2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Data segment */
    gdt_set_gate (3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* User mode code segment */
    gdt_set_gate (4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* User mode data segment */
    write_tss (5, 0x10, 0x0);

    gdt_flush ((uint32_t) &p_gdt);
    tss_flush ();
}

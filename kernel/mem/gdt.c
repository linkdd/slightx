#include <mem.h>

/* The GDT (Global Descriptor Table) containes 256 descriptors.
 * We use only 2. The first descriptor must be NULL, the second
 * is for the code segment and the third for the data segment
 */

struct gdt_entry gdt[3];
struct gdt_ptr gp;

/* Init a descriptor */
void gdt_set_gate (int i, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
	gdt[i].base_low    = (base & 0xFFFF);
	gdt[i].base_middle = (base >> 16) & 0xFF;
	gdt[i].base_high   = (base >> 24) & 0xFF;

	gdt[i].limit_low   = (limit & 0xFFFF);
	gdt[i].granularity = (limit >> 16) & 0x0F;

	gdt[i].granularity |= gran & 0xF0;
	gdt[i].access = access;
	return;
}

void init_gdt (void)
{
	gp.limit = (sizeof (struct gdt_entry) * 6) - 1;
	gp.base  = (unsigned int) &gdt;

	gdt_set_gate (0, 0, 0, 0, 0);                /* NULL gate */
	gdt_set_gate (1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Code segment */
	gdt_set_gate (2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Data segment */
	
	gdt_flush ();
	return;
}

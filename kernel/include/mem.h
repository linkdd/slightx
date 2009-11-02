#ifndef __MEM_H
#define __MEM_H

#define NULL  ((void *) 0)

struct gdt_entry
{
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_middle;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__ ((packed));

struct gdt_ptr
{
	unsigned short limit;
	unsigned int base;
} __attribute__ ((packed));

void gdt_set_gate (int i, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
void init_gdt (void);

void init_paging (void);

#endif /* __MEM_H */

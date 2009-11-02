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

struct idt_entry
{
	unsigned short base_low;
	unsigned short selector;
	unsigned char unused;
	unsigned char flags;
	unsigned char base_high;
} __attribute__ ((packed));

struct idt_ptr
{
	unsigned short limit;
	unsigned int base;
} __attribute__ ((packed));

struct registers_t
{
	unsigned int ds; /* data segment selector */
	unsigned int edi, esi, ebp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
	unsigned int int_no, err_code; /* Interrupt number and error code (if applicable) */
	unsigned int eip, cs, eflags, useresp, ss; /* pushed by the processor automatically */
};

extern void gdt_flush (void);
void gdt_set_gate (int i, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
void init_gdt (void);

void init_paging (void);

#endif /* __MEM_H */

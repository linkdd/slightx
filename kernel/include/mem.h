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

void init_paging (void);

void init_gdt (void);
void gdt_set_gate (int i, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
extern void gdt_flush (void);

void init_pic (void);
void init_idt (void);
void idt_set_gate (unsigned char i, unsigned int base, unsigned short sel, unsigned char flags);

/* ASM ISR handlers */
extern void isr0  (void);
extern void isr1  (void);
extern void isr2  (void);
extern void isr3  (void);
extern void isr4  (void);
extern void isr5  (void);
extern void isr6  (void);
extern void isr7  (void);
extern void isr8  (void);
extern void isr9  (void);
extern void isr10 (void);
extern void isr11 (void);
extern void isr12 (void);
extern void isr13 (void);
extern void isr14 (void);
extern void isr15 (void);
extern void isr16 (void);
extern void isr17 (void);
extern void isr18 (void);
extern void isr19 (void);
extern void isr20 (void);
extern void isr21 (void);
extern void isr22 (void);
extern void isr23 (void);
extern void isr24 (void);
extern void isr25 (void);
extern void isr26 (void);
extern void isr27 (void);
extern void isr28 (void);
extern void isr29 (void);
extern void isr30 (void);
extern void isr31 (void);
extern void irq0  (void);
extern void irq1  (void);
extern void irq2  (void);
extern void irq3  (void);
extern void irq4  (void);
extern void irq5  (void);
extern void irq6  (void);
extern void irq7  (void);
extern void irq8  (void);
extern void irq9  (void);
extern void irq10 (void);
extern void irq11 (void);
extern void irq12 (void);
extern void irq13 (void);
extern void irq14 (void);
extern void irq15 (void);

#endif /* __MEM_H */

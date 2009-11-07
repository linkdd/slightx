#include <mem.h>
#include <isr.h>

struct idt_entry idt[256];
struct idt_ptr p_idt;

void idt_set_gate (unsigned char i, unsigned int base, unsigned short sel, unsigned char flags)
{
	idt[i].base_low  = base & 0xFFFF;
	idt[i].base_high = (base >> 16) & 0xFFFF;
	idt[i].selector  = sel;
	idt[i].unused    = 0;
	idt[i].flags     = flags;
}

void init_idt (void)
{
	int i;

	p_idt.limit = sizeof (struct idt_entry) * 256 - 1;
	p_idt.base = (unsigned int) &idt;

	for (i = 0; i < 256; ++i)
		idt_set_gate (i, 0, 0, 0);

	idt_set_gate (0x00, (unsigned int) isr0,  0x08, 0x8E);
	idt_set_gate (0x01, (unsigned int) isr1,  0x08, 0x8E);
	idt_set_gate (0x02, (unsigned int) isr2,  0x08, 0x8E);
	idt_set_gate (0x03, (unsigned int) isr3,  0x08, 0x8E);
	idt_set_gate (0x04, (unsigned int) isr4,  0x08, 0x8E);
	idt_set_gate (0x05, (unsigned int) isr5,  0x08, 0x8E);
	idt_set_gate (0x06, (unsigned int) isr6,  0x08, 0x8E);
	idt_set_gate (0x07, (unsigned int) isr7,  0x08, 0x8E);
	idt_set_gate (0x08, (unsigned int) isr8,  0x08, 0x8E);
	idt_set_gate (0x09, (unsigned int) isr9,  0x08, 0x8E);
	idt_set_gate (0x0A, (unsigned int) isr10, 0x08, 0x8E);
	idt_set_gate (0x0B, (unsigned int) isr11, 0x08, 0x8E);
	idt_set_gate (0x0C, (unsigned int) isr12, 0x08, 0x8E);
	idt_set_gate (0x0D, (unsigned int) isr13, 0x08, 0x8E);
	idt_set_gate (0x0E, (unsigned int) isr14, 0x08, 0x8E);
	idt_set_gate (0x0E, (unsigned int) isr15, 0x08, 0x8E);
	idt_set_gate (0x10, (unsigned int) isr16, 0x08, 0x8E);
	idt_set_gate (0x11, (unsigned int) isr17, 0x08, 0x8E);
	idt_set_gate (0x12, (unsigned int) isr18, 0x08, 0x8E);
	idt_set_gate (0x13, (unsigned int) isr19, 0x08, 0x8E);
	idt_set_gate (0x14, (unsigned int) isr20, 0x08, 0x8E);
	idt_set_gate (0x15, (unsigned int) isr21, 0x08, 0x8E);
	idt_set_gate (0x16, (unsigned int) isr22, 0x08, 0x8E);
	idt_set_gate (0x17, (unsigned int) isr23, 0x08, 0x8E);
	idt_set_gate (0x18, (unsigned int) isr24, 0x08, 0x8E);
	idt_set_gate (0x19, (unsigned int) isr25, 0x08, 0x8E);
	idt_set_gate (0x1A, (unsigned int) isr26, 0x08, 0x8E);
	idt_set_gate (0x1B, (unsigned int) isr27, 0x08, 0x8E);
	idt_set_gate (0x1C, (unsigned int) isr28, 0x08, 0x8E);
	idt_set_gate (0x1D, (unsigned int) isr29, 0x08, 0x8E);
	idt_set_gate (0x1E, (unsigned int) isr30, 0x08, 0x8E);
	idt_set_gate (0x1F, (unsigned int) isr31, 0x08, 0x8E);

	idt_set_gate (0x20, (unsigned int) irq0,  0x08, 0x8E);
	idt_set_gate (0x21, (unsigned int) irq1,  0x08, 0x8E);
	idt_set_gate (0x22, (unsigned int) irq2,  0x08, 0x8E);
	idt_set_gate (0x23, (unsigned int) irq3,  0x08, 0x8E);
	idt_set_gate (0x24, (unsigned int) irq4,  0x08, 0x8E);
	idt_set_gate (0x25, (unsigned int) irq5,  0x08, 0x8E);
	idt_set_gate (0x26, (unsigned int) irq6,  0x08, 0x8E);
	idt_set_gate (0x27, (unsigned int) irq7,  0x08, 0x8E);
	idt_set_gate (0x28, (unsigned int) irq8,  0x08, 0x8E);
	idt_set_gate (0x29, (unsigned int) irq9,  0x08, 0x8E);
	idt_set_gate (0x2A, (unsigned int) irq10, 0x08, 0x8E);
	idt_set_gate (0x2B, (unsigned int) irq11, 0x08, 0x8E);
	idt_set_gate (0x2C, (unsigned int) irq12, 0x08, 0x8E);
	idt_set_gate (0x2D, (unsigned int) irq13, 0x08, 0x8E);
	idt_set_gate (0x2E, (unsigned int) irq14, 0x08, 0x8E);
	idt_set_gate (0x2F, (unsigned int) irq15, 0x08, 0x8E);

	asm ("lidtl (p_idt)");
}

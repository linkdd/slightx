#include <mem.h>
#include <interrupts.h>

extern struct exceptions_t exceptions[];
struct idt_entry idt[256];
struct idt_ptr p_idt;

void idt_set_gate (unsigned char i, unsigned int base, unsigned short sel, unsigned char flags, int user)
{
	idt[i].base_low  = base & 0xFFFF;
	idt[i].base_high = (base >> 16) & 0xFFFF;
	idt[i].selector  = sel;
	idt[i].unused    = 0;
	idt[i].flags     = (user ? (flags | 0x60) : flags);
}

void init_idt (void)
{
	int i;

	p_idt.limit = sizeof (struct idt_entry) * 256 - 1;
	p_idt.base = (unsigned int) &idt;

	for (i = 0; i < 256; ++i)
		idt_set_gate (i, (unsigned int) isr_handler, 0x08, 0x8E, 0);

	for (i = 0; exceptions[i].msg != NULL; ++i)
		idt_set_gate (exceptions[i].i, exceptions[i].handler, 0x08, 0x08E, 0);

	asm ("lidtl (p_idt)");
}

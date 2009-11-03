#include <mem.h>
#include <interrupts.h>
#include <screen.h>

struct exceptions_t exceptions[] =
{
	{ 0,  "Divide by zero",                                  Fault,       (unsigned int) isr0 },
	{ 1,  "Debug exception",                                 TrapOrFault, (unsigned int) isr1 },
	{ 2,  "Non-Maskable Interrupt (NMI)",                    Trap,        (unsigned int) isr2 },
	{ 3,  "Breakpoint (INT 3)",                              Trap,        (unsigned int) isr3 },
	{ 4,  "Overflow (INTO with EFlags[OF] set)",             Trap,        (unsigned int) isr4 },
	{ 5,  "Bound exception (BOUND on out-of-bounds access)", Trap,        (unsigned int) isr5 },
	{ 6,  "Invalid Opcode",                                  Trap,        (unsigned int) isr6 },
	{ 7,  "FPU not available",                               Trap,        (unsigned int) isr7 },
	{ 8,  "Double Fault",                                    Abort,       (unsigned int) isr8 },
	{ 9,  "Coprocessor Segment Overrun",                     Abort,       (unsigned int) isr9 },
	{ 10, "Invalid TSS",                                     Fault,       (unsigned int) isr10 },
	{ 11, "Segment not present",                             Fault,       (unsigned int) isr11 },
	{ 12, "Stack exception",                                 Fault,       (unsigned int) isr12 },
	{ 13, "General Protection",                              TrapOrFault, (unsigned int) isr13 },
	{ 14, "Page Fault",                                      Fault,       (unsigned int) isr14 },
	{ 16, "Floating-point error",                            Fault,       (unsigned int) isr16 },
	{ 17, "Alignment Check",                                 Fault,       (unsigned int) isr17 },
	{ 18, "Machine Check",                                   Abort,       (unsigned int) isr16 },
	{ 0, NULL, None, 0 }
};

void isr_handler (struct registers_t regs)
{
	print ("unhandled interrupt: ");
	print_dec (regs.int_no);
	print ("\n");
	return;
}

void isr_error_handler (struct registers_t regs)
{
	if (regs.int_no < 32)
	{
		print ("Error: ");
		print (exceptions[regs.int_no].msg);
		print ("\nSystem halted.\n");
		asm ("hlt");
	}
	return;
}


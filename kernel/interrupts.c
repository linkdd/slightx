#include <mem.h>
#include <interrupts.h>
#include <screen.h>

void isr_handler (struct registers_t regs)
{
	print ("unhandled interrupt: ");
	print_dec (regs.int_no);
	print ("\n");
	return;
}

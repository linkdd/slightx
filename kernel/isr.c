#include <mem.h>
#include <asm/asm.h>
#include <isr.h>
#include <screen.h>

static isr_t interrupt_handlers[256] = { 0 };

void isr_handler (struct registers_t regs)
{
	if (interrupt_handlers[regs.int_no] != 0)
	{
		isr_t handler = interrupt_handlers[regs.int_no];
		handler (regs);
	}

	return;
}

void irq_handler (struct registers_t regs)
{
	/* Send an EOI (end of interrupt) signal to the PICs.
	 * If this interrupt involved the slave.
	 */
	if (regs.int_no >= 40)
		outb (0xA0, 0x20);

	/* Send reset signal to master. (As well as slave, if necessary). */
	outb (0x20, 0x20);

	if (interrupt_handlers[regs.int_no] != 0)
	{
		isr_t handler = interrupt_handlers[regs.int_no];
		handler (regs);
	}
		
	return;
}

void set_interrupt_handler (char n, isr_t handler)
{
	interrupt_handlers[n] = handler;
}

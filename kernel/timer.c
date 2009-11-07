#include <sys/io.h>
#include <timer.h>
#include <isr.h>
#include <screen.h>

unsigned int tick = 0;

static void cb_timer (struct registers_t regs)
{
	tick++;
	printk ("Tick: %d\n", tick);
	return;
}

void init_timer (unsigned int frequency)
{
	unsigned int divisor;
	unsigned char l, h;

	set_interrupt_handler (0x20, &cb_timer); /* register timer callback */

	/* The value we send to the PIT is the value to divide it's input clock
	 * (1193180 Hz) by, to get our required frequency. Important to note is
	 * that the divisor must be small enough to fit into 16-bits.
	 */
	divisor = 1193180 / frequency;

	outb (0x43, 0x36);
	l = (unsigned char) (divisor & 0xFF);
	h = (unsigned char) ((divisor >> 8) & 0xFF);

	/* Send the frequency divisor */
	outb (0x40, l);
	outb (0x40, h);

	return;
}

#include <isr.h>
#include <sys/io.h>
#include <screen.h>
#include <util.h>

isr_t interrupt_handlers[256];

void set_interrupt_handler (uint8_t n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}

/* Called from our ASM interrupt handler stub */
void isr_handler (struct registers_t regs)
{
    /* This line is important. When the processor extends the 8-bit interrupt number
     * to a 32bit value, it sign-extends, not zero extends. So if the most significant
     * bit (0x80) is set, regs.int_no will be very large (about 0xFFFFFF80).
     */
    uint8_t int_no = regs.int_no & 0xFF;

    if (interrupt_handlers[int_no] != 0)
    {
        isr_t handler = interrupt_handlers[int_no];
        handler (&regs);
    }
    else
    {
        printk ("unhandled interrupt: %x\n", regs.int_no);
    }
}

/* Called from our ASM interrupt handler stub */
void irq_handler (struct registers_t regs)
{
    /* Send an EOI (end of interrupt) signal to the PICs.
     * If this interrupt involved the slave
     */
    if (regs.int_no >= 40)
        outb (0xA0, 0x20); /* Send reset signal to slave */
    outb (0x20, 0x20); /* Send reset signal to master. (As well as slave, if necessary). */

    if (interrupt_handlers[regs.int_no] != 0)
    {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler (&regs);
    }
}

#ifndef __ISR_H
#define __ISR_H

struct registers_t
{
	unsigned int ds;                                /* data segment selector */
	unsigned int edi, esi, ebp, ebx, edx, ecx, eax; /* pushed by 'pusha' */
	unsigned int int_no, err_code;                  /* Interrupt number and error code (if applicable) */
	unsigned int eip, cs, eflags, useresp, ss;      /* pushed by the processor automatically */
};

typedef void (*isr_t)(struct registers_t);

void isr_handler (struct registers_t regs);
void irq_handler (struct registers_t regs);

void set_interrupt_handler (char n, isr_t handler);

#endif /* __ISR_H */

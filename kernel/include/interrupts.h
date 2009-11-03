#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H

extern void isr0 (void);
extern void isr1 (void);
extern void isr2 (void);
extern void isr3 (void);
extern void isr4 (void);
extern void isr5 (void);
extern void isr6 (void);
extern void isr7 (void);
extern void isr8 (void);
extern void isr9 (void);
extern void isr10 (void);
extern void isr11 (void);
extern void isr12 (void);
extern void isr13 (void);
extern void isr14 (void);
extern void isr16 (void);
extern void isr17 (void);
extern void isr18 (void);

void isr_handler (struct registers_t regs);

#endif /* __INTERRUPTS_H */

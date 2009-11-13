#include <mem/idt.h>
#include <util.h>

struct idt_entry_t idt_entries[256];
struct idt_t p_idt;

static void idt_set_gate (int num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;

    idt_entries[num].sel    = sel;
    idt_entries[num].unused = 0;
    idt_entries[num].flags  = flags | 0x60;
}

void init_idt (void)
{
    p_idt.limit = sizeof (struct idt_entry_t) * 256 - 1;
    p_idt.base  = (uint32_t) &idt_entries;

    memset (&idt_entries, 0, sizeof (struct idt_entry_t) * 256);

    /* exceptions */
    idt_set_gate (0x00, (uint32_t) isr0,  0x08, 0x8E);
    idt_set_gate (0x01, (uint32_t) isr1,  0x08, 0x8E);
    idt_set_gate (0x02, (uint32_t) isr2,  0x08, 0x8E);
    idt_set_gate (0x03, (uint32_t) isr3,  0x08, 0x8E);
    idt_set_gate (0x04, (uint32_t) isr4,  0x08, 0x8E);
    idt_set_gate (0x05, (uint32_t) isr5,  0x08, 0x8E);
    idt_set_gate (0x06, (uint32_t) isr6,  0x08, 0x8E);
    idt_set_gate (0x07, (uint32_t) isr7,  0x08, 0x8E);
    idt_set_gate (0x08, (uint32_t) isr8,  0x08, 0x8E);
    idt_set_gate (0x09, (uint32_t) isr9,  0x08, 0x8E);
    idt_set_gate (0x0A, (uint32_t) isr10, 0x08, 0x8E);
    idt_set_gate (0x0B, (uint32_t) isr11, 0x08, 0x8E);
    idt_set_gate (0x0C, (uint32_t) isr12, 0x08, 0x8E);
    idt_set_gate (0x0D, (uint32_t) isr13, 0x08, 0x8E);
    idt_set_gate (0x0E, (uint32_t) isr14, 0x08, 0x8E);
    idt_set_gate (0x0F, (uint32_t) isr15, 0x08, 0x8E);
    idt_set_gate (0x10, (uint32_t) isr16, 0x08, 0x8E);
    idt_set_gate (0x11, (uint32_t) isr17, 0x08, 0x8E);
    idt_set_gate (0x12, (uint32_t) isr18, 0x08, 0x8E);
    idt_set_gate (0x13, (uint32_t) isr19, 0x08, 0x8E);
    idt_set_gate (0x14, (uint32_t) isr20, 0x08, 0x8E);
    idt_set_gate (0x15, (uint32_t) isr21, 0x08, 0x8E);
    idt_set_gate (0x16, (uint32_t) isr22, 0x08, 0x8E);
    idt_set_gate (0x17, (uint32_t) isr23, 0x08, 0x8E);
    idt_set_gate (0x18, (uint32_t) isr24, 0x08, 0x8E);
    idt_set_gate (0x19, (uint32_t) isr25, 0x08, 0x8E);
    idt_set_gate (0x1A, (uint32_t) isr26, 0x08, 0x8E);
    idt_set_gate (0x1B, (uint32_t) isr27, 0x08, 0x8E);
    idt_set_gate (0x1C, (uint32_t) isr28, 0x08, 0x8E);
    idt_set_gate (0x1D, (uint32_t) isr29, 0x08, 0x8E);
    idt_set_gate (0x1E, (uint32_t) isr30, 0x08, 0x8E);
    idt_set_gate (0x1F, (uint32_t) isr31, 0x08, 0x8E);

    /* IRQs */
    idt_set_gate (0x20, (uint32_t) irq0,  0x08, 0x8E);
    idt_set_gate (0x21, (uint32_t) irq1,  0x08, 0x8E);
    idt_set_gate (0x22, (uint32_t) irq2,  0x08, 0x8E);
    idt_set_gate (0x23, (uint32_t) irq3,  0x08, 0x8E);
    idt_set_gate (0x24, (uint32_t) irq4,  0x08, 0x8E);
    idt_set_gate (0x25, (uint32_t) irq5,  0x08, 0x8E);
    idt_set_gate (0x26, (uint32_t) irq6,  0x08, 0x8E);
    idt_set_gate (0x27, (uint32_t) irq7,  0x08, 0x8E);
    idt_set_gate (0x28, (uint32_t) irq8,  0x08, 0x8E);
    idt_set_gate (0x29, (uint32_t) irq9,  0x08, 0x8E);
    idt_set_gate (0x2A, (uint32_t) irq10, 0x08, 0x8E);
    idt_set_gate (0x2B, (uint32_t) irq11, 0x08, 0x8E);
    idt_set_gate (0x2C, (uint32_t) irq12, 0x08, 0x8E);
    idt_set_gate (0x2D, (uint32_t) irq13, 0x08, 0x8E);
    idt_set_gate (0x2E, (uint32_t) irq14, 0x08, 0x8E);
    idt_set_gate (0x2F, (uint32_t) irq15, 0x08, 0x8E);

    /* syscall */
    idt_set_gate (0x80, (uint32_t) isr128, 0x08, 0x8E);

    idt_flush ((uint32_t)  &p_idt);
}

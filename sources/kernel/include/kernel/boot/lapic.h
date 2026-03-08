#pragma once

#include <klibc/types.h>

#include <kernel/boot/int.h>


#define APIC_REG_ID               0x020
#define APIC_REG_EOI              0x0B0
#define APIC_REG_SVR              0x0F0
#define APIC_REG_ICR_LOW          0x300
#define APIC_REG_ICR_HIGH         0x310
#define APIC_REG_LVT_TIMER        0x320
#define APIC_REG_INITCNT          0x380
#define APIC_REG_CURRCNT          0x390
#define APIC_REG_DIV              0x3E0

#define APIC_SVR_SW_ENABLE        (1 << 8)
#define APIC_LVT_MASK             (1 << 16)
#define APIC_LVT_PERIODIC         (1 << 17)

#define APIC_BASE_ENABLE          (1 << 11)

#define IDT_GATE_LAPIC_TIMER      0xE0
#define IDT_GATE_LAPIC_PANIC      0xFE
#define IDT_GATE_LAPIC_SPURIOUS   0xFF

#define LAPIC_DIVIDE_BY_128       0xB

#define LAPIC_TIMER_TICK_MS       1


void lapic_write(u32 reg, u32 value);
u32  lapic_read (u32 reg);
void lapic_eoi  (void);

void lapic_calibrate      (void);
void lapic_configure_timer(void);

void lapic_send_panic_ipi(void);

void lapic_timer_handler   (interrupt_frame *iframe);
void lapic_panic_handler   (interrupt_frame *iframe);
void lapic_spurious_handler(interrupt_frame *iframe);

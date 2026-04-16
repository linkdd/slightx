#include <klibc/io/log.h>

#include <kernel/boot/lapic.h>
#include <kernel/boot/madt.h>
#include <kernel/boot/idt.h>

#include <kernel/cpu/msr.h>
#include <kernel/cpu/ioport.h>
#include <kernel/cpu/mp.h>

#include <kernel/halt.h>


extern void lapic_timer_stub   (void);
extern void lapic_panic_stub   (void);
extern void lapic_spurious_stub(void);

static u32 lapic_ticks_per_ms;


void lapic_write(u32 reg, u32 value) {
  uptr lapic_base = (uptr)madt_lapic_base();
  uptr addr       = lapic_base + (uptr)reg;
  *(volatile u32 *)addr = value;
  (void)*(volatile u32 *)addr; // flush
}


u32 lapic_read(u32 reg) {
  uptr lapic_base = (uptr)madt_lapic_base();
  uptr addr       = lapic_base + (uptr)reg;
  return *(volatile u32 *)addr;
}


void lapic_eoi(void) {
  lapic_write(APIC_REG_EOI, 0);
}


static void lapic_enable_sw(void) {
  u64 apic_base = rdmsr(IA32_APIC_BASE) | APIC_BASE_ENABLE;
  wrmsr(IA32_APIC_BASE, apic_base);

  lapic_write(APIC_REG_SVR, APIC_SVR_SW_ENABLE | IDT_GATE_LAPIC_SPURIOUS);
}


static void lapic_program_timer_periodic(u32 ticks) {
  lapic_write(APIC_REG_LVT_TIMER, APIC_LVT_MASK | IDT_GATE_LAPIC_TIMER);
  lapic_write(APIC_REG_DIV,       LAPIC_DIVIDE_BY_128);
  lapic_write(APIC_REG_INITCNT,   ticks > 0 ? ticks : 1);
  lapic_write(APIC_REG_LVT_TIMER, APIC_LVT_PERIODIC | IDT_GATE_LAPIC_TIMER);
}


void lapic_calibrate(void) {
  const u16 pit_reload = 11932; // ~10ms at 1'193'182 Hz

  lapic_write(APIC_REG_LVT_TIMER, APIC_LVT_MASK);
  lapic_write(APIC_REG_DIV,       LAPIC_DIVIDE_BY_128);
  lapic_write(APIC_REG_INITCNT,   0xFFFFFFFF);

  // Program PIT to mode 0 (rate generator), low/high byte, channel 2
  outb(0x43, 0b10110000);  // Channel 2, access mode lobyte/hibyte, mode 0, binary
  u8 spk = inb(0x61);

  // Ensure gate is enabled (bit1=1), speaker off (bit0=0)
  outb(0x61, (spk & ~(1 << 0)) | (1 << 1));
  // Load count, starting the countdown
  outb(0x42, pit_reload & 0xFF);
  outb(0x42, pit_reload >> 8);

  // Poll OUT bit (bit5) until it goes to 1, with a simple timeout
  for (u32 i = 0; i < 2'000'000; i++) {
    if (inb(0x61) & (1 << 5)) {
      u32 after   = lapic_read(APIC_REG_CURRCNT);
      u32 elapsed = 0xFFFFFFFF - after;
      if (elapsed == 0) elapsed = 1;

      lapic_ticks_per_ms = elapsed / 10;
      if (lapic_ticks_per_ms == 0) lapic_ticks_per_ms = 1;

      return;
    }
  }

  // fallback
  lapic_ticks_per_ms = 1'000'000;
}


void lapic_send_panic_ipi(void) {
  // All-excluding-self shorthand (dest shorthand = 0b11), fixed delivery, vector = IDT_GATE_LAPIC_PANIC
  lapic_write(APIC_REG_ICR_HIGH, 0);
  lapic_write(APIC_REG_ICR_LOW,  (0b11 << 18) | IDT_GATE_LAPIC_PANIC);
}


void lapic_configure_timer(void) {
  idt_set_gate(IDT_GATE_LAPIC_TIMER,    (u64)lapic_timer_stub,    0x8E, 0x01);
  idt_set_gate(IDT_GATE_LAPIC_PANIC,    (u64)lapic_panic_stub,    0x8E, 0x01);
  idt_set_gate(IDT_GATE_LAPIC_SPURIOUS, (u64)lapic_spurious_stub, 0x8E, 0x01);

  lapic_enable_sw();
  lapic_program_timer_periodic(lapic_ticks_per_ms * LAPIC_TIMER_TICK_MS);
}


void lapic_timer_handler(interrupt_frame *iframe) {
  (void)iframe;

  scheduler_tick(1'000'000 * LAPIC_TIMER_TICK_MS);
  lapic_eoi();
}


void lapic_panic_handler(interrupt_frame *iframe) {
  (void)iframe;
  __asm__ volatile("cli");
  halt();
}


void lapic_spurious_handler(interrupt_frame *iframe) {
  (void)iframe;
  lapic_eoi();
}

#include <kernel/boot/exc.h>
#include <kernel/boot/idt.h>
#include <kernel/halt.h>

#include <klibc/mem/str.h>
#include <klibc/sync/lock.h>
#include <klibc/io/log.h>


static const str exc_names[] = {
  str_literal("Division by zero"),
  str_literal("Single-step interrupt"),
  str_literal("Non maskable interrupt"),
  str_literal("Breakpoint"),
  str_literal("Overflow"),
  str_literal("Bound Range Exceeded"),
  str_literal("Invalid Opcode"),
  str_literal("Coprocessor not available"),
  str_literal("Double Fault"),
  str_literal("Coprocessor Segment Overrun"),
  str_literal("Invalid Task State Segment"),
  str_literal("Segment Not Present"),
  str_literal("Stack Segment Fault"),
  str_literal("General Protection Fault"),
  str_literal("Page Fault"),
  str_literal("(reserved)"),
  str_literal("x87 Floating Point Exception"),
  str_literal("Alignment Check"),
  str_literal("Machine Check"),
  str_literal("SIMD Floating Point Exception"),
  str_literal("Virtualization Exception"),
  str_literal("Control Protection Exception"),
  str_literal("(reserved)"),
  str_literal("(reserved)"),
  str_literal("(reserved)"),
  str_literal("(reserved)"),
  str_literal("(reserved)"),
  str_literal("(reserved)"),
  str_literal("(reserved)"),
  str_literal("(reserved)"),
  str_literal("(reserved)"),
  str_literal("(reserved)"),
};

static interrupt_cb exc_callbacks[IDT_NUM_EXCEPTIONS] = {};
static spinlock     exc_lock                          = {};


void exc_init(void) {
  spinlock_init(&exc_lock);
}


void exc_handler(interrupt_frame *iframe) {
  interrupt_cb          cb = exc_get_callback(iframe->interrupt);
  interrupt_controlflow cf = INTERRUPT_CONTROLFLOW_HALT;

  if (cb != NULL) {
    cb(iframe, &cf);
  }
  else {
    klog(
      "[UNHANDLED EXCEPTION] %x - %s",
      iframe->interrupt,
      exc_names[iframe->interrupt]
    );
    klog(
      "  CR0[%x] CR2[%x] CR3[%x] CR4[%x]",
      iframe->cr0, iframe->cr2, iframe->cr3, iframe->cr4
    );
    klog(
      "  RAX[%x] RBX[%x] RCX[%x] RDX[%x]",
      iframe->rax, iframe->rbx, iframe->rcx, iframe->rdx
    );
    klog(
      "  RSI[%x] RDI[%x] RSP[%x] RBP[%x]",
      iframe->rsi, iframe->rdi, iframe->rsp, iframe->rbp
    );
    klog(
      "  R8[%x] R9[%x] R10[%x] R11[%x]",
      iframe->r8, iframe->r9, iframe->r10, iframe->r11
    );
    klog(
      "  R12[%x] R13[%x] R14[%x] R15[%x]",
      iframe->r12, iframe->r13, iframe->r14, iframe->r15
    );
    klog(
      "  CS[%x] RIP[%x] CODE[%x] RFLAGS[%x]",
      iframe->cs, iframe->rip, iframe->err_code, iframe->rflags
    );
  }

  if (cf == INTERRUPT_CONTROLFLOW_HALT) {
    asm("cli");
    halt();
  }
}


interrupt_cb exc_get_callback(u8 exc) {
  interrupt_cb cb;

  spinlock_acquire(&exc_lock);
  cb = exc_callbacks[exc];
  spinlock_release(&exc_lock);

  return cb;
}


void exc_set_callback(u8 exc, interrupt_cb cb) {
  spinlock_acquire(&exc_lock);
  exc_callbacks[exc] = cb;
  spinlock_release(&exc_lock);
}


void exc_clear_callback(u8 exc) {
  spinlock_acquire(&exc_lock);
  exc_callbacks[exc] = NULL;
  spinlock_release(&exc_lock);
}

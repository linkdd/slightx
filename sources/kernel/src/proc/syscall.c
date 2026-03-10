#include <klibc/io/log.h>
#include <klibc/assert.h>

#include <kernel/boot/gdt.h>
#include <kernel/cpu/msr.h>
#include <kernel/cpu/mp.h>
#include <kernel/proc/syscall.h>
#include <kernel/proc/thread.h>
#include <kernel/drivers/console.h>


extern void syscall_entry_stub(void);


// MARK: - implementations

static i64 sysc_exit(syscall_frame *frame) {
  i32 exit_code = (i32)frame->rdi;
  thread_exit_from_task(exit_code);
  unreachable();
}


static i64 sysc_mmap(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  void            *addr   = (void*)          frame->rdi;
  usize            length = (usize)          frame->rsi;
  task_mmap_flags  flags  = (task_mmap_flags)frame->rdx;

  void *result = task_mmap(cur, addr, length, flags);

  return (i64)(uptr)result;
}


static i64 sysc_munmap(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  void  *addr   = (void*)frame->rdi;
  usize  length = (usize)frame->rsi;

  task_munmap(cur, addr, length);

  return 0;
}


static i64 sysc_puts(syscall_frame *frame) {
  char  *buf = (char*)frame->rdi;
  usize  len = (usize)frame->rsi;

  str s = { .data = buf, .length = len };
  console_write(s);

  return (i64)len;
}


// MARK: - table

static syscall_fn syscall_table[SYSC__COUNT] = {
  [SYSC_EXIT]   = sysc_exit,
  [SYSC_MMAP]   = sysc_mmap,
  [SYSC_MUNMAP] = sysc_munmap,
  [SYSC_PUTS]   = sysc_puts,
};


// MARK: - setup

void syscall_init(void) {}


void syscall_load(void) {
  // Enable SCE (System Call Extensions) in EFER
  u64 efer = rdmsr(IA32_EFER);
  efer |= IA32_EFER_SCE;
  wrmsr(IA32_EFER, efer);

  u64 star = ((u64)GDT_SEGMENT(GDT_SEG_IDX_KDATA) << 48)
           | ((u64)GDT_SEGMENT(GDT_SEG_IDX_KCODE) << 32);
  wrmsr(IA32_STAR, star);
  wrmsr(IA32_LSTAR, (u64)syscall_entry_stub);
  wrmsr(IA32_FMASK, (1 << 9) | (1 << 10));
}


// MARK: - dispatcher

void syscall_handler(syscall_frame *frame) {
  u64 syscall_nr = frame->rax;

  if (syscall_nr >= SYSC__COUNT || syscall_table[syscall_nr] == NULL) {
    klog("[syscall] unknown syscall %x", syscall_nr);
    frame->rax = (u64)(-1LL);
    return;
  }

  frame->rax = (u64)syscall_table[syscall_nr](frame);
}

#include <klibc/io/log.h>
#include <klibc/assert.h>

#include <kernel/boot/gdt.h>

#include <kernel/cpu/msr.h>
#include <kernel/cpu/mp.h>

#include <kernel/proc/syscall.h>
#include <kernel/proc/spawn.h>
#include <kernel/proc/task.h>


extern void syscall_entry_stub(void);


// MARK: - implementations

static i64 sysc_exit(syscall_frame *frame) {
  i32 exit_code = (i32)frame->rdi;
  task_exit(exit_code);
  unreachable();
}


static i64 sysc_spawn(syscall_frame *frame) {
  const void                   *data = (const void*)                   frame->rdi;
  usize                         len   = (usize)                        frame->rsi;
  const task_user_startup_info *info  = (const task_user_startup_info*)frame->rdx;

  const_span binary = make_const_span(data, len);
  return (i64)spawn_user_task(binary, info);
}


static i64 sysc_join(syscall_frame *frame) {
  task_join((tid)frame->rdi);
  return 0;
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


static i64 sysc_send(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  cap_id      cap  = (cap_id)     frame->rdi;
  const void *data = (const void*)frame->rsi;
  usize       len  = (usize)      frame->rdx;

  const_span msg = make_const_span(data, len);

  cap_obj *obj = cap_table_get(&cur->capabilities, cap, CAP_RIGHT_SEND);
  if (obj == NULL || obj->ops->send == NULL) return -1;

  return obj->ops->send(obj, msg);
}


static i64 sysc_call(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  cap_id      cap       = (cap_id)     frame->rdi;
  const void *req_data  = (const void*)frame->rsi;
  usize       req_len   = (usize)      frame->rdx;
  void       *resp_data = (void*)      frame->r10;
  usize       resp_len  = (usize)      frame->r8;

  const_span req  = make_const_span(req_data,  req_len);
  span       resp = make_span      (resp_data, resp_len);

  cap_obj *obj = cap_table_get(&cur->capabilities, cap, CAP_RIGHT_CALL);
  if (obj == NULL || obj->ops->call == NULL) return -1;

  return obj->ops->call(obj, req, resp);
}


static i64 sysc_capctl(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  cap_id cap = (cap_id)frame->rdi;
  u64    cmd = (u64)   frame->rsi;
  uptr   arg = (uptr)  frame->rdx;

  cap_obj *obj = cap_table_get(&cur->capabilities, cap, CAP_RIGHT_CTL);
  if (obj == NULL || obj->ops->ctl == NULL) return -1;

  return obj->ops->ctl(obj, cmd, arg);
}


// MARK: - table

static syscall_fn syscall_table[SYSC__COUNT] = {
  [SYSC_EXIT]   = sysc_exit,
  [SYSC_SPAWN]  = sysc_spawn,
  [SYSC_JOIN]   = sysc_join,

  [SYSC_MMAP]   = sysc_mmap,
  [SYSC_MUNMAP] = sysc_munmap,

  [SYSC_SEND]   = sysc_send,
  [SYSC_CALL]   = sysc_call,
  [SYSC_CAPCTL] = sysc_capctl,
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

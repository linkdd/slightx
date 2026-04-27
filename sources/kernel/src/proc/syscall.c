#include <klibc/io/log.h>
#include <klibc/assert.h>

#include <kernel/boot/gdt.h>

#include <kernel/cpu/msr.h>
#include <kernel/cpu/mp.h>

#include <kernel/proc/syscall.h>
#include <kernel/proc/spawn.h>
#include <kernel/proc/task.h>
#include <kernel/proc/abi.h>


extern void syscall_entry_stub(void);


// MARK: - validation
//
// On x86_64 we require user pointers to live in the canonical lower-half
// (0 .. 0x0000_8000_0000_0000). This is a coarse range check: it does not
// verify that the pages are actually mapped/readable/writable. A real fault
// during access will still take a page fault and the task will be killed.
//
// The goal here is to make it impossible for userspace to coerce the kernel
// into following a kernel pointer it supplied via a syscall register. Without
// this check, a malicious task can pass e.g. 0xFFFFFFFF80000000 and have the
// kernel happily memcpy from / to its own image.

#define USER_VA_END 0x0000800000000000ULL

static inline bool is_user_range_ok(uptr addr, usize len) {
  if (len == 0)                 return true;  // empty range, nothing to deref
  if (addr + len < addr)        return false; // wrap-around
  if (addr + len > USER_VA_END) return false; // crosses canonical hole
  return true;
}


static inline bool is_user_ptr_ok(const void *p, usize len) {
  return is_user_range_ok((uptr)p, len);
}


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

  if (!is_user_ptr_ok(data, len)) {
    return -EFAULT;
  }
  if (info != NULL) {
    if (!is_user_ptr_ok(info, sizeof(*info)))                                    return -EFAULT;
    if (!is_user_ptr_ok(info->args.items,    info->args.count    * sizeof(str))) return -EFAULT;
    if (!is_user_ptr_ok(info->envvars.items, info->envvars.count * sizeof(str))) return -EFAULT;

    for (usize i = 0; i < info->args.count; i++) {
      if (!is_user_ptr_ok(info->args.items[i].data, info->args.items[i].length)) return -EFAULT;
    }

    for (usize i = 0; i < info->envvars.count; i++) {
      if (!is_user_ptr_ok(info->envvars.items[i].data, info->envvars.items[i].length)) return -EFAULT;
    }
  }

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

  // For FIXED mappings the address is meaningful and must be in user space.
  // For non-FIXED mappings addr is a hint; we only require that, if non-NULL,
  // it fits in user space (so the kernel never returns a kernel address).
  if (length == 0)                                           return -EINVAL;
  if (addr != NULL && !is_user_range_ok((uptr)addr, length)) return -EFAULT;

  void *result = task_mmap(cur, addr, length, flags);

  return (i64)(uptr)result;
}


static i64 sysc_munmap(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  void  *addr   = (void*)frame->rdi;
  usize  length = (usize)frame->rsi;

  if (length == 0)                           return -EINVAL;
  if (!is_user_range_ok((uptr)addr, length)) return -EFAULT;

  task_munmap(cur, addr, length);

  return 0;
}


static i64 sysc_capread(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  cap_id  cap  = (cap_id)frame->rdi;
  void   *data = (void*) frame->rsi;
  usize   len  = (usize) frame->rdx;

  if (!is_user_ptr_ok(data, len)) return -EFAULT;

  span msg = make_span(data, len);

  cap_obj *obj = cap_table_get(&cur->capabilities, cap, CAP_RIGHT_READ);
  if (obj == NULL)            return -EINVAL;
  if (obj->ops->read == NULL) return -ENOSUP;

  return obj->ops->read(obj, msg);
}


static i64 sysc_capwrite(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  cap_id      cap  = (cap_id)     frame->rdi;
  const void *data = (const void*)frame->rsi;
  usize       len  = (usize)      frame->rdx;

  if (!is_user_ptr_ok(data, len)) return -EFAULT;

  const_span msg = make_const_span(data, len);

  cap_obj *obj = cap_table_get(&cur->capabilities, cap, CAP_RIGHT_WRITE);
  if (obj == NULL)             return -EINVAL;
  if (obj->ops->write == NULL) return -ENOSUP;

  return obj->ops->write(obj, msg);
}


static i64 sysc_capinvoke(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  cap_id      cap       = (cap_id)     frame->rdi;
  const void *req_data  = (const void*)frame->rsi;
  usize       req_len   = (usize)      frame->rdx;
  void       *resp_data = (void*)      frame->r10;
  usize       resp_len  = (usize)      frame->r8;

  if (!is_user_ptr_ok(req_data,  req_len))  return -EFAULT;
  if (!is_user_ptr_ok(resp_data, resp_len)) return -EFAULT;

  const_span req  = make_const_span(req_data,  req_len);
  span       resp = make_span      (resp_data, resp_len);

  cap_obj *obj = cap_table_get(&cur->capabilities, cap, CAP_RIGHT_INVOKE);
  if (obj == NULL)              return -EINVAL;
  if (obj->ops->invoke == NULL) return -ENOSUP;

  return obj->ops->invoke(obj, req, resp);
}


static i64 sysc_capmap(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  cap_id   cap             = (cap_id)frame->rdi;
  void    *addr            = (void*) frame->rsi;
  usize    size            = (usize) frame->rdx;
  u8       flags           = (u8)    frame->r10;
  void   **mapped_addr = (void**)frame->r8;

  // addr is a hint or a fixed user VA: must be in user space if non-NULL.
  if (size == 0)                                                                 return -EINVAL;
  if (addr != NULL && !is_user_range_ok((uptr)addr, size))                       return -EFAULT;
  if (mapped_addr != NULL && !is_user_ptr_ok(mapped_addr, sizeof(*mapped_addr))) return -EFAULT;

  cap_obj *obj = cap_table_get(&cur->capabilities, cap, CAP_RIGHT_MAP);
  if (obj == NULL)           return -EINVAL;
  if (obj->ops->map == NULL) return -ENOSUP;

  return obj->ops->map(obj, addr, size, flags, mapped_addr);
}


static i64 sysc_capctl(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  cap_id cap = (cap_id)frame->rdi;
  u64    cmd = (u64)   frame->rsi;
  uptr   arg = (uptr)  frame->rdx;

  cap_obj *obj = cap_table_get(&cur->capabilities, cap, CAP_RIGHT_CTL);
  if (obj == NULL)           return -EINVAL;
  if (obj->ops->ctl == NULL) return -ENOSUP;

  return obj->ops->ctl(obj, cmd, arg);
}


static i64 sysc_caprelease(syscall_frame *frame) {
  percpu_data *cpu = mp_get_percpu_data();
  task        *cur = cpu->scheduler.current;

  cap_id cap = (cap_id)frame->rdi;
  cap_table_del(&cur->capabilities, cap);

  return 0;
}


// MARK: - table

static syscall_fn syscall_table[SYSC__COUNT] = {
  [SYSC_EXIT]   = sysc_exit,
  [SYSC_SPAWN]  = sysc_spawn,
  [SYSC_JOIN]   = sysc_join,

  [SYSC_MMAP]   = sysc_mmap,
  [SYSC_MUNMAP] = sysc_munmap,

  [SYSC_CAPREAD]    = sysc_capread,
  [SYSC_CAPWRITE]   = sysc_capwrite,
  [SYSC_CAPINVOKE]  = sysc_capinvoke,
  [SYSC_CAPMAP]     = sysc_capmap,
  [SYSC_CAPCTL]     = sysc_capctl,
  [SYSC_CAPRELEASE] = sysc_caprelease,
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

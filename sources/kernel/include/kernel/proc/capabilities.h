#pragma once

#include <klibc/types.h>
#include <klibc/sync/lock.h>
#include <klibc/mem/alloc.h>
#include <klibc/mem/span.h>


typedef u32 cap_id;

typedef enum : u32 {
  CAP_RIGHT_NONE = 0,

  CAP_RIGHT_SEND = 1 << 0,
  CAP_RIGHT_CALL = 1 << 1,
  CAP_RIGHT_CTL  = 1 << 2,

  CAP_RIGHT_ALL = 0xFFFFFFFF,
} cap_rights;

typedef struct cap_obj cap_obj;
typedef struct cap_ops cap_ops;

typedef struct cap_slot cap_slot;
struct cap_slot {
  cap_obj    *obj;
  cap_rights  rights;
  u32         flags;

  u16 gen;
  u16 next_free;
};

typedef struct cap_table cap_table;
struct cap_table {
  allocator a;

  cap_slot *slots;
  usize     capacity;

  u16 next_unused;
  u16 free_head;

  spinlock  lock;
};

struct cap_obj {
  const cap_ops *ops;
  atomic_uint    refcount;
  u32            flags;
};

struct cap_ops {
  void (*release)(cap_obj *self);

  i64 (*send)(cap_obj *obj, const_span msg);
  i64 (*call)(cap_obj *obj, const_span msg, span reply);
  i64 (*ctl) (cap_obj *obj, u64 cmd, uptr arg);
};


cap_obj *cap_obj_incref(cap_obj *self);
void     cap_obj_decref(cap_obj *self);


void cap_table_init  (cap_table *self, allocator a, usize initial_capacity);
void cap_table_deinit(cap_table *self);

cap_id   cap_table_add(cap_table *self, cap_obj *obj, cap_rights rights, u32 flags);
cap_obj *cap_table_get(cap_table *self, cap_id id, cap_rights required_rights);
void     cap_table_del(cap_table *self, cap_id id);

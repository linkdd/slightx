#include <klibc/assert.h>
#include <klibc/mem/bytes.h>

#include <kernel/proc/capabilities.h>


static constexpr u16 CAP_INVALID_INDEX = 0xFFFF;


static inline cap_id cap_id_make(u16 index, u16 gen) {
  return (cap_id)(((u32)gen << 16) | index);
}

static inline u16 cap_id_get_index(cap_id id) {
  return (u16)(id & 0xFFFF);
}

static inline u16 cap_id_get_gen(cap_id id) {
  return (u16)(id >> 16);
}


// MARK: - obj
void cap_obj_init(cap_obj *self, const cap_ops *ops, allocator a) {
  assert(self != NULL);
  assert(ops  != NULL);

  atomic_init(&self->refcount, 1);
  self->ops   = ops;
  self->flags = 0;
  self->a     = a;
}

cap_obj *cap_obj_incref(cap_obj *self) {
  assert(self != NULL);
  atomic_fetch_add_explicit(&self->refcount, 1, memory_order_relaxed);
  return self;
}


void cap_obj_decref(cap_obj *self) {
  assert(self != NULL);
  if (atomic_fetch_sub_explicit(&self->refcount, 1, memory_order_acq_rel) == 1) {
    if (self->ops->release != NULL) {
      self->ops->release(self);
    }
    deallocate(self->a, self, sizeof(cap_obj));
  }
}


// MARK: - table
static void cap_table__ensure_capacity(cap_table *self) {
  assert(self != NULL);

  if (self->next_unused < self->capacity) return;
  if (self->capacity == 0xFFFF)           return;

  usize old_capacity = self->capacity;
  usize new_capacity = (old_capacity == 0) ? 1 : (old_capacity * 2);
  if (new_capacity > 0xFFFF) new_capacity = 0xFFFF;

  self->slots = reallocate(
    self->a,
    self->slots,
    old_capacity * sizeof(cap_slot),
    new_capacity * sizeof(cap_slot)
  );
  self->capacity = new_capacity;

  for (u16 i = old_capacity; i < new_capacity; i++) {
    self->slots[i].obj       = NULL;
    self->slots[i].rights    = 0;
    self->slots[i].flags     = 0;
    self->slots[i].gen       = 1;
    self->slots[i].next_free = (i < new_capacity - 1) ? (i + 1) : self->free_head;
  }
}


void cap_table_init(cap_table *self, allocator a, usize initial_capacity) {
  assert(self != NULL);
  assert(initial_capacity <= 0xFFFF);

  self->a           = a;
  self->slots       = allocate_v(a, initial_capacity, sizeof(cap_slot));
  self->capacity    = initial_capacity;
  self->next_unused = 0;
  self->free_head   = (initial_capacity > 0) ? 0 : CAP_INVALID_INDEX;

  for (usize i = 0; i < initial_capacity; i++) {
    self->slots[i].obj       = NULL;
    self->slots[i].rights    = 0;
    self->slots[i].flags     = 0;
    self->slots[i].gen       = 1;
    self->slots[i].next_free = CAP_INVALID_INDEX;
  }

  spinlock_init(&self->lock);
}


void cap_table_deinit(cap_table *self) {
  assert(self != NULL);

  for (usize i = 0; i < self->capacity; i++) {
    cap_slot *slot = &self->slots[i];
    if (slot->obj != NULL) {
      cap_obj_decref(slot->obj);
    }
  }

  deallocate(self->a, self->slots, self->capacity * sizeof(cap_slot));
  memset(self, 0, sizeof(cap_table));
}


cap_id cap_table_add(cap_table *self, cap_obj *obj, cap_rights rights, u32 flags) {
  assert(self != NULL);
  assert(obj  != NULL);

  spinlock_acquire(&self->lock);

  u16 idx = CAP_INVALID_INDEX;

  if (self->free_head != CAP_INVALID_INDEX) {
    idx = self->free_head;
    self->free_head = self->slots[idx].next_free;
  }
  else {
    cap_table__ensure_capacity(self);
    assert(self->next_unused < self->capacity);

    idx = self->next_unused;
    self->next_unused++;
  }

  cap_slot *slot = &self->slots[idx];
  assert(slot->obj == NULL);

  slot->obj       = cap_obj_incref(obj);
  slot->rights    = rights;
  slot->flags     = flags;
  slot->next_free = CAP_INVALID_INDEX;

  cap_id id = cap_id_make(idx, slot->gen);

  spinlock_release(&self->lock);

  return id;
}


cap_obj *cap_table_get(cap_table *self, cap_id id, cap_rights required_rights) {
  assert(self != NULL);

  u16 idx = cap_id_get_index(id);
  u16 gen = cap_id_get_gen(id);

  spinlock_acquire(&self->lock);

  if (idx >= self->capacity) {
    spinlock_release(&self->lock);
    return NULL;
  }

  cap_slot *slot = &self->slots[idx];
  if (slot->obj == NULL) {
    spinlock_release(&self->lock);
    return NULL;
  }

  if (slot->gen != gen) {
    spinlock_release(&self->lock);
    return NULL;
  }

  if ((slot->rights & required_rights) != required_rights) {
    spinlock_release(&self->lock);
    return NULL;
  }

  cap_obj *obj = cap_obj_incref(slot->obj);

  spinlock_release(&self->lock);
  return obj;
}


void cap_table_del(cap_table *self, cap_id id) {
  assert(self != NULL);

  u16 idx = cap_id_get_index(id);
  u16 gen = cap_id_get_gen(id);

  spinlock_acquire(&self->lock);

  if (idx >= self->capacity) {
    spinlock_release(&self->lock);
    return;
  }

  cap_slot *slot = &self->slots[idx];
  if (slot->obj == NULL || slot->gen != gen) {
    spinlock_release(&self->lock);
    return;
  }

  cap_obj *obj = slot->obj;

  slot->obj       = NULL;
  slot->rights    = 0;
  slot->flags     = 0;
  slot->gen      += 1;
  if (slot->gen == 0) slot->gen = 1;

  slot->next_free = self->free_head;
  self->free_head = idx;

  spinlock_release(&self->lock);

  cap_obj_decref(obj);
}

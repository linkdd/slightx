#include <klibc/assert.h>

#include <kernel/proc/capabilities/console.h>
#include <kernel/drivers/console.h>


static void console_cap_release(cap_obj *obj) {
  console_cap *cap = (console_cap *)obj;

  if (cap != NULL) {
    deallocate(cap->a, cap, sizeof(console_cap));
  }
}


static i64 console_cap_write(cap_obj *obj, const_span msg) {
  console_cap *cap = (console_cap *)obj;
  assert(cap != NULL);

  console_write((str){
    .data     = (char *)msg.data,
    .length   = msg.size,
    .capacity = msg.size,
    .owned    = false,
  });

  return (i64)msg.size;
}


static const cap_ops console_cap_ops = {
  .release = console_cap_release,
  .read    = NULL,
  .write   = console_cap_write,
  .invoke  = NULL,
  .map     = NULL,
  .ctl     = NULL,
};


cap_obj *make_console_cap(allocator a) {
  console_cap *cap = allocate(a, sizeof(console_cap));
  if (cap == NULL) return NULL;

  atomic_init(&cap->base.refcount, 1);
  cap->base.ops   = &console_cap_ops;
  cap->base.flags = 0;

  cap->a = a;

  return (cap_obj *)cap;
}

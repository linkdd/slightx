#include <klibc/assert.h>

#include <kernel/proc/capabilities/console.h>
#include <kernel/drivers/console.h>


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
  .release = NULL,
  .read    = NULL,
  .write   = console_cap_write,
  .invoke  = NULL,
  .map     = NULL,
  .ctl     = NULL,
};


cap_obj *make_console_cap(allocator a) {
  console_cap *cap = allocate(a, sizeof(console_cap));
  cap_obj_init(&cap->base, &console_cap_ops, a);
  return (cap_obj *)cap;
}

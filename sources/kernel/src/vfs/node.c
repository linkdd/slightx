#include <kernel/vfs/node.h>
#include <klibc/assert.h>


vfs_node_ref vfs_node_incref(vfs_node_ref node) {
  assert(node != NULL);
  atomic_fetch_add_explicit(&node->refcount, 1, memory_order_relaxed);
  return node;
}


void vfs_node_decref(vfs_node_ref node) {
  assert(node != NULL);

  u32 prev = atomic_fetch_sub_explicit(&node->refcount, 1, memory_order_acq_rel);
  if (prev == 1) {
    atomic_thread_fence(memory_order_acquire);
    if (node->release != NULL) node->release(node);
  }
}

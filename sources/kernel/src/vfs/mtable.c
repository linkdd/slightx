#include "mtable.h"


static vfs_mtable g_mtable = {};


void vfs_mtable_init(void) {
  spinlock_init(&g_mtable.lock);
  g_mtable.head = NULL;
}


void vfs_mtable_lock(void) {
  spinlock_acquire(&g_mtable.lock);
}


void vfs_mtable_unlock(void) {
  spinlock_release(&g_mtable.lock);
}


vfs_mtable_entry *vfs_mtable_get_head(void) {
  return g_mtable.head;
}


void vfs_mtable_set_head(vfs_mtable_entry *entry) {
  g_mtable.head = entry;
}

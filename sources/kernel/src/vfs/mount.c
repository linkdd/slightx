#include <kernel/vfs/mount.h>
#include <kernel/mem/heap.h>

#include <klibc/assert.h>

#include "mtable.h"
#include "path.h"


// MARK: - mount

RESULT(UNIT, vfs_err) vfs_mount(str path, vfs_node *root) {
  assert(path.length > 0);
  assert(path.data[0] == '/');
  assert(root != NULL);

  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  vfs_mtable_lock();

  if (root->type != VFS_NODETYPE_DIRECTORY) {
    vfs_mtable_unlock();

    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOTDIR);
  }

  if (root->mounted != NULL) {
    vfs_mtable_unlock();

    return (RESULT(UNIT, vfs_err)) ERR(VFS_EBUSY);
  }

  for (
    vfs_mtable_entry *entry = vfs_mtable_get_head();
    entry != NULL;
    entry = entry->next
  ) {
    if (str_equal(entry->mount.path, normpath)) {
      vfs_mtable_unlock();

      return (RESULT(UNIT, vfs_err)) ERR(VFS_EEXISTS);
    }
  }

  allocator a = heap_allocator();

  vfs_mtable_entry *entry = allocate(a, sizeof(vfs_mtable_entry));
  entry->mount.path = str_clone(a, normpath);
  entry->mount.root = vfs_node_incref(root);

  root->mounted = &entry->mount;

  if (vfs_mtable_get_head() == NULL) {
    entry->prev = NULL;
    entry->next = NULL;
    vfs_mtable_set_head(entry);
  }
  else {
    entry->prev       = NULL;
    entry->next       = vfs_mtable_get_head();
    entry->next->prev = entry;
    vfs_mtable_set_head(entry);
  }

  vfs_mtable_unlock();

  return (RESULT(UNIT, vfs_err)) OK({});
}


// MARK: - umount

RESULT(UNIT, vfs_err) vfs_umount(str path) {
  assert(path.length > 0);
  assert(path.data[0] == '/');

  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  vfs_mtable_lock();

  vfs_mtable_entry *entry = NULL;

  for (
    entry = vfs_mtable_get_head();
    entry != NULL;
    entry = entry->next
  ) {
    if (str_equal(entry->mount.path, normpath)) {
      break;
    }
  }

  if (entry == NULL) {
    vfs_mtable_unlock();

    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOTFOUND);
  }

  entry->mount.root->mounted = NULL;
  vfs_node_decref(entry->mount.root);

  if (entry->prev != NULL) entry->prev->next = entry->next;
  else                     vfs_mtable_set_head(entry->next);
  if (entry->next != NULL) entry->next->prev = entry->prev;

  allocator a = heap_allocator();

  str_free  (a, &entry->mount.path);
  deallocate(a, entry, sizeof(vfs_mtable_entry));

  vfs_mtable_unlock();

  return (RESULT(UNIT, vfs_err)) OK({});
}

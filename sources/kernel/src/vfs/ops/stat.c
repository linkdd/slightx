#include <klibc/assert.h>

#include <kernel/vfs/node-ops.h>

#include "../path.h"


RESULT(vfs_stat_desc, vfs_err) vfs_stat(str path) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(vfs_stat_desc, vfs_err)) ERR(VFS_EINVAL);
  }

  auto node = vfs_lookup(normpath, false);
  if (!node.is_ok) {
    return (RESULT(vfs_stat_desc, vfs_err)) ERR(node.err);
  }

  vfs_node_ref self = node.ok;

  if (self->ops == NULL || self->ops->stat == NULL) {
    vfs_node_decref(self);
    return (RESULT(vfs_stat_desc, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = self->ops->stat(self);
  vfs_node_decref(self);
  return res;
}

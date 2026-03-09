#include <klibc/assert.h>

#include <kernel/vfs/node-ops.h>

#include "../path.h"


RESULT(UNIT, vfs_err) vfs_chown(str path, u32 uid, u32 gid) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(UNIT, vfs_err)) ERR(VFS_EINVAL);
  }

  auto node = vfs_lookup(normpath, false);
  if (!node.is_ok) {
    return (RESULT(UNIT, vfs_err)) ERR(node.err);
  }

  vfs_node_ref self = node.ok;

  if (self->ops == NULL || self->ops->chown == NULL) {
    vfs_node_decref(self);
    return (RESULT(UNIT, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = self->ops->chown(self, uid, gid);
  vfs_node_decref(self);
  return res;
}

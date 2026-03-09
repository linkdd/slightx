#include <klibc/assert.h>

#include <kernel/vfs/node-ops.h>

#include "../path.h"


RESULT(str, vfs_err) vfs_readlink(str path, allocator a) {
  char pathbuf[VFS_PATH_MAX] = {};
  str  normpath              = vfs_normalize_path(path, pathbuf);
  if (normpath.data == NULL) {
    return (RESULT(str, vfs_err)) ERR(VFS_EINVAL);
  }

  auto node = vfs_lookup(normpath, true);
  if (!node.is_ok) {
    return (RESULT(str, vfs_err)) ERR(node.err);
  }

  vfs_node_ref self = node.ok;

  if (self->type != VFS_NODETYPE_SYMLINK) {
    return (RESULT(str, vfs_err)) ERR(VFS_EINVAL);
  }

  if (self->ops == NULL || self->ops->readlink == NULL) {
    vfs_node_decref(self);
    return (RESULT(str, vfs_err)) ERR(VFS_ENOSYS);
  }

  auto res = self->ops->readlink(self, a);
  vfs_node_decref(self);
  return res;
}
